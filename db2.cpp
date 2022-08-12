#include <Windows.h>
#include <sqlcli1.h>
#include <stdio.h>
#include <tchar.h>
#include <winuser.h>
#include <string>
#include <iostream>
// cl db2.cpp -IC:\rong\libs\db2_libs\CLI9\include C:\rong\libs\db2_libs\CLI9\win32\db2cli.lib User32.lib C:\rong\libs\db2_libs\CLI9\win32\db2api.lib
using namespace std;
// from https://www.daimajiaoliu.com/daima/486f9a5ee9003e4
//  1 全局变量声明
SQLHENV henv = SQL_NULL_HENV;
SQLHDBC hdbc = SQL_NULL_HDBC;
SQLHSTMT hstmt = SQL_NULL_HSTMT;
SQLRETURN rc = SQL_SUCCESS;
SQLCHAR sqlstmt[200];

// 2 分配 Environment 和 Connection handle henv=0;
bool initEnvAndHandle()
{
    /*
    1 HandleType SQLSMALLINT 要分配的 handle 类型 ,
    包括三种： SQL_HANDLE_ENV : Environment handle ；
              SQL_HANDLE_DBC : Connection handle ；
              SQL_HANDLE_STMT: Stmt handle 。
    2 Inputhandle SQLHANDLE 与当前要分配的 handle 所对应的上一级 handle ：
     分配 Environment handle 时 : 指定 SQL_NULL_HANDLE ；
     分配 Connection handle 时 : 指定对应的 Environment handle 变量；
     分配 Stmt handle 时，指定对应的 Connection handle 变量。
    3 OutputHandlePtr SQLHANDLE * 作为分配结果的 handle 指针，指向新分配完毕的 handle 的变量。

    注意：每个程序中只能有一个 Environment handle ，但是却可以分配多个 Connection handle 。
    除了使用 SQLAllocHandle() ，DB2 ODBC 还支持使用 ODBC 1.0 API ：SQLAllocEnv() 、SQLAllocConnect()
    和 SQLAllocStmt() 来分配 Environment ，Connection 及 Stmt handle 。
    为了程序的可维护性和可移植性，推荐用户使用 SQLAllocHandle() 来统一分配 handle 。
    */
    rc = 0;
    printf("\nSQLAllocHandle - allocate ENV handle ");
    rc = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_ENV, (SQLHANDLE)SQL_NULL_HANDLE, (SQLHANDLE *)&henv);
    printf("\nDMLDDLM-henv=%i\n", henv);
    if (rc != SQL_SUCCESS)
    {
        printf("\nAllocate environment handle error\n");
        return false;
    }
    hdbc = 0;
    printf("\nSQLAllocHandle - allocate connection handle ");
    rc = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_DBC, (SQLHANDLE)henv, (SQLHANDLE *)&hdbc);
    printf("\nDMLDDLM-hdbc=%i\n", hdbc);
    if (rc != SQL_SUCCESS)
    {
        printf("\nAllocate connection handle error\n");
        return false;
    }

    return true;
}

// 3.设置和查询全局属性：
// DB2 ODBC 使用 SQLSetEnvAttr() 和 SQLGetEnvAttr() 来设置和查询全局属性。
//一个设置并查询 Environment handle 属性的例子如例 2.3 所示： 例 2.3 设置并查询 Environment handle 属性

bool initEnvAttr()
{
    /*
    例 2.3 使用 SQLSetEnvAttr() 设置 DB2 ODBC 在数据库操作时以 NULL 作为输出字符串的终结符，并通过 SQLGetEnvAttr() 查询设置的结果。
    SQLSetEnvAttr() 的参数描述如表 2.2 所示，SQLGetEnvAttr() 的参数描述如表 2.3 所示： 表 2.2 SQLSetEnvAttr() 参数描述


1 EnvironmentHandle SQLHENV 当前程序的 Environment handle 变量 2 Attribute SQLINTEGER 属性名称，
具体包括： 当前 ODBC 版本 SQL_ATTR_ODBC_VERSION ， 输出字符串终结符属性 SQL_ATTR_OUTPUT_NTS ， 连接类型 SQL_ATTR_CONNECTTYPE 等。
更多的属性名称和详细用法请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》
3 ValuePtr SQLPOINTER 指向属性值变量的指针 4 StringLength SQLINTEGER 当 value 为字符类型时，value 的字节长度 当 value 不为字符类型时，则忽略此参数
表 2.3 SQLGetEnvAttr() 参数描述

1 EnvironmentHandle SQLHENV 当前程序的 Environment handle 变量 2 Attribute SQLINTEGER 属性名称，具体包括： 当前 ODBC 版本 SQL_ATTR_ODBC_VERSION ，
 输出字符串终结符属性 SQL_ATTR_OUTPUT_NTS ， 连接类型 SQL_ATTR_CONNECTTYPE 等。
  更多的属性名称和详细用法请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》 3 ValuePtr SQLPOINTER 指向返回属性值变量的指针
   4 BufferLength SQLINTEGER ValuePtr 指针指向的 buffer 的最大长度 5 StringLengthPtr SQLINTEGER * 指向包含 ValuePtr 长度变量的指针

设置和查询连接属性：DB2 ODBC 推荐使用 ODBC3.0 API SQLSetConnectAttr() 和 SQLGetConnectAttr() 来设置和获得连接属性。
一个设置和查询连接属性的例子如例 2.4 所示：
    */
    SQLINTEGER output_nts;
    rc = SQLSetEnvAttr(henv, SQL_ATTR_OUTPUT_NTS, (SQLPOINTER)SQL_TRUE, 0);
    if (rc != SQL_SUCCESS)
    {
        printf("\nSet environment handle attribute error\n");
        return false;
    }
    rc = SQLGetEnvAttr(henv, SQL_ATTR_OUTPUT_NTS, &output_nts, 0, 0);
    if (rc != SQL_SUCCESS)
    {
        printf("\nGet environment handle attribute error\n");
        return false;
    }
    printf("\nNull Termination of Output strings is: ");
    if (output_nts == SQL_TRUE)
        printf("True\n");
    else
        printf("False\n");
    return true;
}

// 4 设置和查询连接属性
bool initConnect()
{
    /*
    使用 SQLSetConnectAttr() 将 Connection handle 设置为手动提交事务，作用范围为当前连接，并使用 SQLGetConnectAttr 查询设置结果。
    此外，DB2 ODBC 还支持使用 ODBC 1.0 API SQLSetConnectOption() 和 SQLGetConnectOption() 进行 Connection 属性的设置和查询，
    但出于可移植性和扩展性的考虑，同样不做推荐。建议使用 SQLSetConnectAttr() 和 SQLGetConnectAttr() 进行相关的操作。
    例 2.4 中 SQLSetConnectAttr() 的参数描述如表 2.4 所示，SQLGetConnectAttr() 的参数说明如表 2.5 所示： 表 2.4 SQLSetConnectAttr() 参数描述


1 EnvironmentHandle SQLHDBC 当前程序的 Connection handle 变量 2 Attribute SQLINTEGER 属性名称，
具体包括： 连接类型 SQL_ATTR_CONNECTTYPE ， 事务自动提交 SQL_ATTR_AUTOCOMMIT 等。
更多的属性名称和详细用法请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》 3 ValuePtr SQLPOINTER 指向属性值变量的指针 4 StringLength SQLINTEGER 当 value 为 Integer 类型时，则忽略此参数 当 value 为字符类型时： i. 如果 Attribute 是 ODBC 定义的属性，应为 value 的长度； ii. 如果是 IBM 扩展所定义的属性，并且 value 是以 nul-terminated 的字符串，则此处可以为 value 的长度或者 SQL_NTS
表 2.5 SQLGetConnectAttr() 参数描述

1 EnvironmentHandle SQLHDBC 当前程序的 Connection handle 变量 2 Attribute SQLINTEGER 属性名称，
具体包括： 连接类型 SQL_ATTR_CONNECTTYPE ， 事务自动提交 SQL_ATTR_AUTOCOMMIT 等。
更多的属性名称和详细用法请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》
3 ValuePtr SQLPOINTER 指向返回的属性值变量的指针 4 BufferLength SQLINTEGER ValuePtr 指向的 buffer 的字节长度，
且取决于以下条件： 当 value 为 Integer 类型时，则忽略此参数； 当 value 为字符类型时：
i. 如果 Attribute 是 ODBC 定义的属性，应为 value 的长度；
ii. 如果是 ibm 扩展所定义的属性，并且 value 是以 nul-terminated 的字符串，
则此处可以为 value 的长度或者 SQL_NTS 5 StringLengthPtr SQLINTEGER * 指向包含 ValuePtr 长度变量的指针
    */
    // SQL_AUTOCOMMIT_ON SQL_AUTOCOMMIT_OFF?
    SQLINTEGER autocommit;
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (bool *)SQL_AUTOCOMMIT_ON, SQL_NTS);
    if (rc != SQL_SUCCESS)
    {
        printf("\nset connection handle attribute error\n");
        return false;
    }
    rc = SQLGetConnectAttr(hdbc, SQL_AUTOCOMMIT, &autocommit, 0, NULL);
    if (rc != SQL_SUCCESS)
    {
        printf("\nget connection handle attribute error\n");
        return false;
    }
    printf("\nAutocommit is: ");
    if (autocommit == SQL_AUTOCOMMIT_ON)
        printf("ON\n");
    else
        printf("OFF\n");
    return true;
}

//连接到数据源
bool connect()
{
    /*
    1 hdbc SQLHDBC 连接数据源的 connection handle 变量
    2 hwnd SQLHWND 此参数未使用，通常指定为 NULL
    3 szConnStrIn SQLCHAR * 连接字符串，可以在其中指定连接的详细信息
    4 cbConnStrIn SQLSMALLINT szConnStrIn 的字节数
    5 szConnStrOut SQLCHAR * 指向包含连接字符串的 buffer 的指针变量
    6 cbConnStrOutMax SQLSMALLINT szConnStrOut 所指向的 buffer 的最大字节数
    7 pcbConnStrOut SQLSMALLINT * 指向包含连接字符串中有效字节数量的 buffer 的指针变量
    8 fDriverCompletion SQLUSMALLINT 运行时是否提示用户需要更多信息的变量，DB2 ODBC 只支持不提示信息即 SQL_DRIVER_NOPROMPT
    */
    const char *login = "***";
    const char *password = "***!";
    const char *schema = "***";
    string decodedPwd = "***!";

    SQLCHAR driverconnect[100];
    sprintf((char *)driverconnect, (char *)"DSN=FTRONGHU; UID=huangron; PWD=Datafax2021!;");
    rc = SQLDriverConnect(hdbc, NULL, driverconnect, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (rc != SQL_SUCCESS)
    {
        printf("\ndriver connection error\n");
        return false;
    }
    return true;
}

//分配 Stmt handle： 分配并设置 Stmt handle 属性实例
bool initStamtHandle()
{
    printf("\nSQLAllocStmt \n");
    hstmt = 0;
    rc = SQLAllocHandle((SQLSMALLINT)SQL_HANDLE_STMT, (SQLHANDLE)hdbc, (SQLHANDLE *)&hstmt);
    if (rc != SQL_SUCCESS)
        return false;
    printf("\nhstmt=%i\n", hstmt);
    printf("\nSet Stmt handle attributes ");
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_TXN_ISOLATION, (bool *)SQL_TXN_READ_COMMITTED, (SQLINTEGER)0);
    if (rc != SQL_SUCCESS)
        return false;

    string exec = "SET CURRENT SCHEMA POKERS";

    SQLINTEGER len = strlen((const char *)exec.c_str());

    return true;
}
// set schema
bool initSchema()
{
    string exec = "SET CURRENT SCHEMA POKERS";

    SQLINTEGER len = strlen((const char *)exec.c_str());
    rc = SQLExecDirect(hstmt, (SQLCHAR *)exec.c_str(), len);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
    {
        std::cout << "\nError >>SET CURRENT SCHEMA.\n";
        return false;
    }
    return true;
}
// 执行数据库操作 1。
bool ExecSqlQuery()
{
    auto retcode = SQLExecDirect(hstmt, (SQLCHAR *)"SELECT * FROM USERS", SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        cout << "error >> no data\n";
        return false;
    }
    else
    {
        cout << "--data--------\n";
        SQLSMALLINT columns; /* number of columns in result-set */
        SQLNumResultCols(hstmt, &columns);
        SQLCHAR buf[5][64];
        SQLINTEGER indicator[5];

        for (int i = 0; i < columns; i++)
        {
            SQLBindCol(hstmt, i + 1, SQL_C_CHAR,
                       buf[i], sizeof(buf[i]), &indicator[i]);
        }

        while (SQLFetch(hstmt) == SQL_SUCCESS)
        {
            cout << "------------------\n";
            for (int i = 0; i < columns; i++)
            {
                if (indicator[i] == SQL_NULL_DATA)
                {
                    printf("  Column %u : NULL\n", i);
                }
                else
                {
                    printf("  Column %u : %s\n", i, buf[i]);
                }
            }
        }
    }
    return true;
}

bool execInsert()
{
    auto retcode = SQLExecDirect(hstmt, (SQLCHAR *)"insert into ACTIVITIES (USER_ID,TIME,TYPE) values (2, '2020-11-02',1)", SQL_NTS);
    if (retcode != SQL_SUCCESS)
    {
        cout << "\nerror >> execInsert\n";
        return false;
    }
    return true;
}
// Create a table
bool ExecSqlCreateTable()
{

    /*
SQLExecDirect() 参数说明

 1 hstmt SQLHSTMT Stmt handle 变量
 2 sqlstmt SQLCHAR * 指向数据库操作变量的指针
 3 StringLength SQLINTEGER 指定 sqlstmt 的字节长度 , 或者当 sqlstmt 以 null 为终结符时 , 指定其为 SQL_NTS
*/
    strcpy((char *)sqlstmt, "CREATE TABLE Post6 (user_id BIGINT NOT NULL ,time TIMESTAMP ,message VARCHAR(500), FOREIGN KEY (user_id) REFERENCES USERS(ID) )");
    printf("\nDMLDDLM \n sqlstmt=%s", sqlstmt);
    rc = SQLExecDirect(hstmt, sqlstmt, SQL_NTS);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        return false;

    return true;
}

// 执行数据库操作 1。
bool RepeatSql()
{
    /*
    对于在程序中可能重复执行的操作，通常使用先准备后执行的方法，
    这样只需一次准备多次执行即可，有效地提高了程序的运行效率。
    DB2 ODBC 使用 SQLPrepare() 准备操作，使用 SQLExecute() 来执行对应的操作。
    SQLPrepare() 的参数描述与 SQLExecDirect() 相同，
    SQLExecute() 只有一个参数，即准备完毕的 Stmt handle。
    例 2.8 中，用户在插入数据时直接把数据写死在 sqlstmt 中，导致了该 sqlstmt 只能使用一次，
    如果需要插入多条数据的情况，建立多个对应的 sqlstmt 显然不是一个很好的解决办法。
    在这种情况下，用户可以通过参数绑定的方式将要插入的数据绑定到指定 sqlstmt 中，
    此时，在 sqlstmt 中用通配符"?"来代替所对应的数据，如"INSERT INTO TABLE2A VALUES (?)"，
    这样通过一次 prepare 多次 bind 就可以完成对应的操作，使用起来更加简便。
    DB2 ODBC 使用 SQLBindParameter() 完成参数绑定的过程，
    一个使用 SQLBindParameter() 绑定参数且准备后执行插入数据的例子如例 2.9 所示： 例 2.9 通过绑定参数 , 先准备后执行的方式插入数据
    */

    /*
    DB2 ODBC 通过 SQLBindParameter() 将变量 H1INT4 绑定到 sqlstmt 中的"?"中去，然后通过准备后执行完成插入的功能。当需要继续插入数据时，只需要重新将对应的变量绑定到 hstmt 然后运行 SQLExecute 即可，不需要重新准备。SQLBindParameter() 的参数描述如表 2.10 所示： 表 2.10 SQLBindParameter 参数描述

 1 hstmt SQLHSTMT 进行绑定的 stmt handle 变量；
 2 ipar SQLUSMALLINT 参数通配符的序号，从 1 开始计数；
 3 fParamType SQLSMALLINT 参数类型： SQL_PARAM_INPUT ：输入参数； SQL_PARAM_ONPUT ：
 输出参数，主要用于 store procedure ； SQL_PARAM_INPUT_OUTPUT ：输入输出参数，主要用于 stored procedure ；
 4 fCType SQLSMALLINT 指定参数的 C 的类型，包括 SQL_C_FLOAT, ，SQL_C_LONG, ，SQL_C_SHORT ，SQL_C_TYPE_DATE ，SQL_C_TYPE_TIME 等。
  更多的类型请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》
   5 fSqlType SQLSMALLINT 指定参数的 SQL 类型，包括 SQL_DOUBLE ， SQL_FLOAT ，SQL_GRAPHIC ，SQL_INTEGER 等。
   更多的类型请参看 《 DB2 Version 9.1 for z/OS ODBC Guide and Reference 》
   6 cbColDef SQLUINTEGER 参数的精度； 7 ibScale SQLSMALLINT 参数的 scale ；
   8 rgbValue SQLPOINTER 指向绑定数据的指针；
  9 cbValueMax SQLINTEGER 当绑定参数为 binary 或者 character 时，指定其字节长度；
  10 pcbValue SQLINTEGER * 指向包含绑定数据长度变量的指针；
    */
    long H1INT4;
    long LNH1INT4;
    printf("\nSQLPrepare ");
    strcpy((char *)sqlstmt, "INSERT INTO TABLE2A VALUES(?)");
    printf("\nsqlstmt=%s", sqlstmt);
    rc = SQLPrepare(hstmt, sqlstmt, SQL_NTS);
    if (rc != SQL_SUCCESS)
        return false;
    printf("\nSQLBindParameter \n");
    H1INT4 = 1;
    LNH1INT4 = sizeof(H1INT4);
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &H1INT4, 0, (SQLINTEGER *)&LNH1INT4);
    if (rc != SQL_SUCCESS)
        return false;
    printf("\n SQLExecute ");
    rc = SQLExecute(hstmt);
    if (rc != SQL_SUCCESS)
        return false;
    return true;
}

//处理操作结果： -  使用 SQLFetch() 处理返回结果
bool getResult()
{
    SQLINTEGER H1INT4;
    SQLINTEGER LNH1INT4;
    printf("\nSQLBindCol \n");
    rc = SQLBindCol(hstmt, 1, SQL_C_DEFAULT, (SQLPOINTER)&H1INT4, (SQLINTEGER)sizeof(H1INT4), (SQLINTEGER *)&LNH1INT4);
    if (rc != SQL_SUCCESS)
        return false;
    printf("\n SQLFetch \n");
    rc = SQLFetch(hstmt);
    printf("\nH1INT4 = %i", H1INT4);
    return true;
}

//释放 stmt handle

bool release()
{
    printf("\nSQLFreeHandle() statement handle ");
    rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (rc != SQL_SUCCESS)
        return false;
    return true;
}

int main()
{
    auto ret = initEnvAndHandle();
    if (!ret)
    {
        cout << "Error >> initEnvAndHandle\n";
        return 0;
    }

    ret = initEnvAttr();
    if (!ret)
    {
        cout << "Error >> initEnvAttr\n";
        // return 0;
    }

    ret = initConnect();
    if (!ret)
    {
        cout << "Error >> initConnect\n";
        return 0;
    }

    ret = connect();
    if (!ret)
    {
        cout << "Error >> connect\n";
        return 0;
    }

    ret = initStamtHandle();
    if (!ret)
    {
        cout << "Error >> initStamtHandle\n";
        return 0;
    }
    ret = initSchema();
    if (!ret)
    {
        cout << "Error >> initScheme\n";
        return 0;
    }
    // ret = ExecSqlQuery();
    // if (!ret)
    // {
    //     cout << "Error >> query\n";
    // }
    ret = execInsert();
    if (!ret)
    {
        cout << "Error >> execInsert\n";
    }

    ret = ExecSqlCreateTable();
    if (!ret)
    {
        cout << "Error >> ExecSqlCreateTable\n";
        return 0;
    }

    cout << "--<<Main>>---";
    return 0;
}
