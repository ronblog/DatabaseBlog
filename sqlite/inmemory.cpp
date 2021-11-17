
#include "D:\\3rdLib\\include\\sqlite\\sqlite3.h"
#include <iostream>
using namespace std;
sqlite3 *pDB = NULL;
//#pragma comment (lib, "lib/sqlite3.lib")
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int main() {
  int nRes = sqlite3_open(":memory:", &pDB);
  if (nRes != SQLITE_OK) {
    cout << "Open database fail: " << sqlite3_errmsg(pDB);
  } else
    cout << "Opend" << endl;
  //基本插入

  string strSql = "create table tbl1(one varchar(10), two smallint);";
  char *cErrMsg;
  nRes = sqlite3_exec(pDB, strSql.c_str(), 0, 0, &cErrMsg);

  /* Create SQL statement */
  string sql = "CREATE TABLE COMPANY("
               "ID INT PRIMARY KEY     NOT NULL,"
               "NAME           TEXT    NOT NULL,"
               "AGE            INT     NOT NULL,"
               "ADDRESS        CHAR(50),"
               "SALARY         REAL );";

  /* Execute SQL statement */
  char *zErrMsg = 0;
  int rc = sqlite3_exec(pDB, sql.c_str(), callback, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    fprintf(stdout, "Table created successfully\n");
  }

  /* Create SQL statement */
  sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
        "VALUES (1, 'Paul', 32, 'California', 20000.00 ); "
        "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
        "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "
        "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
        "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );"
        "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
        "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

  /* Execute SQL statement */
  rc = sqlite3_exec(pDB, sql.c_str(), callback, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    fprintf(stdout, "Records created successfully\n");
  }

  /* Create SQL statement */
  sql = "SELECT * from COMPANY";

  /* Execute SQL statement */
  const char *data = "Callback function called";
  rc = sqlite3_exec(pDB, sql.c_str(), callback, (void *)data, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    fprintf(stdout, "Operation done successfully\n");
  }

  //   strSql = "insert into tbl1 values('hello!',10);";
  //   nRes = sqlite3_exec(pDB, strSql.c_str(), 0, 0, &cErrMsg);
  //   strSql = "insert into tbl1 values('Morning!',20);";
  //   nRes = sqlite3_exec(pDB, strSql.c_str(), 0, 0, &cErrMsg);
  //   strSql = "select * from tbl1;";
  //   nRes = sqlite3_exec(pDB, strSql.c_str(), 0, 0, &cErrMsg);

  //基本查询
  sqlite3_close(pDB);
  getchar();
  return 0;
}
static int UserResult(void *NotUsed, int argc, char **argv, char **azColName) {
  for (int i = 0; i < argc; i++) {
    cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << ", ";
  }
  cout << endl;
  return 0;
}