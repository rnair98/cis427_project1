#include "server_functions.h"

static int callback(void *NotUsed, int arc, char ** argv, char **azColName)
{
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void check_entry(int create_entry,char *zErrMsg)
{
  if (create_entry != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    fprintf(stdout, "Entry created.\n");
  }  
}

void init_database(sqlite3 *db)
{
  char *zErrMsg = 0;
  int open_database = sqlite3_open("./cis427_crypto.sqlite",&db);
  int create_entry;
  char *sql;

  if (open_database) {
    fprintf(stderr, "Database not opened: %s\n", sqlite3_errmsg(db));
    return(0);
  } else {
    fprintf(stdout, "Database opened\n");

  /* Create USER Table */

  sql = "CREATE TABLE if NOT EXISTS Users("\
	   "ID int NOT NULL,"\
	   "first_name varchar(255),"\
	   "last_name varchar(255),"\
	   "user_name varchar(255) NOT NULL,"\
	   "password varchar(255),"\
	   "usd_balance DOUBLE NOT NULL,"\
	   "email varchar(255) NOT NULL,"\
	   "PRIMARY KEY (ID));";

  create_entry = sqlite3_exec(db,sql,callback,0,&zErrMsg);
  check_entry(create_entry,zErrMsg);

  /* Create CRYPTOS Table */

  sql = "CREATE TABLE if NOT EXISTS Cryptos("\
	   "ID int NOT NULL,"\
	   "crypto_name varchar(10) NOT NULL,"\
	   "crypto_balance DOUBLE,"\
	   "user_id int,"\
	   "PRIMARY KEY (ID),"\
	   "FOREIGN KEY (user_id) REFERENCES Users (ID));";
  create_entry = sqlite3_exec(db,sql,callback,0,&zErrMsg);
  check_entry(create_entry,zErrMsg);

  /* if USERS doesn't contain 1 person, add a sample user */
  
  sql = "INSERT OR IGNORE INTO Users("\
	   "ID, first_name, last_name, user_name,"\
	   "password, usd_balance, email) "\
	   "VALUES (1,'John','Doe','jdoe','password123',"\
	   "100.00,'jdoe@umich.edu');";
  
  create_entry = sqlite3_exec(db,sql,callback,0,&zErrMsg);
  check_entry(create_entry,zErrMsg);
}

void close_database(sqlite3 *db)
{
  sqlite3_close(db);
}  

