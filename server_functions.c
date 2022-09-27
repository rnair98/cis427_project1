#include <stdio.h>
#include <stdlib.h>
#include "server_functions.h"

int insert_callback(void *NotUsed, int argc, char ** argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++) {
        fprintf(stdout,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    fprintf(stdout  ,"\n");
    return 0;
}

int select_callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++){
      fprintf(stdout,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   fprintf(stdout,"\n");
   return 0;
}

int update_callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++) {
      fprintf(stdout,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   fprintf(stdout,"\n");
   return 0;
}

void check_entry(int sql_execute,char *zErrMsg)
{
    if (sql_execute != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Operation successful.\n");
    }  
}

int init_database(sqlite3 *db, char *zErrMsg,int sql_execute,char *sql)
{
    int open_database = sqlite3_open("./cis427_crypto.sqlite",&db);

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

        sql_execute = sqlite3_exec(db,sql,insert_callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);

        /* Create CRYPTOS Table */

        sql = "CREATE TABLE if NOT EXISTS Cryptos("\
	            "ID int NOT NULL,"\
	            "crypto_name varchar(10) NOT NULL,"\
	            "crypto_balance DOUBLE,"\
	            "user_id int,"\
	            "PRIMARY KEY (ID),"\
	            "FOREIGN KEY (user_id) REFERENCES Users (ID));";
        sql_execute = sqlite3_exec(db,sql,insert_callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);

        /* if USERS doesn't contain 1 person, add a sample user */
  
        sql = "INSERT OR IGNORE INTO Users("\
	            "ID, first_name, last_name, user_name,"\
	            "password, usd_balance, email) "\
	            "VALUES (1,'John','Doe','jdoe','password123',"\
	            "100.00,'jdoe@umich.edu');";
  
        sql_execute = sqlite3_exec(db,sql,insert_callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);
        return open_database;
    }
}

void show_balance(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,int user_id) {
  const char* data = "SELECT OPERATION";
  sprintf(sql,"SELECT usd_balance FROM Users WHERE ID = %d;",user_id);
  sql_execute = sqlite3_exec(db,sql,select_callback,(void*)data,&zErrMsg);
  check_entry(sql_execute,zErrMsg);
}

void buy_crypto(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,const char* crypto_name,double crypto_amount\
                ,double crypto_price, int user_id) {
    const char* select_data = "SELECT OPERATION";
    const char* update_data = "UPDATE OPERATION";
    double user_usd_balance = 0.0;
    double user_crypto_balance = 0.0;

    // get user's usd balance
    sprintf(sql,"SELECT usd_balance FROM Users WHERE ID = %d;",user_id);
    sql_execute = sqlite3_exec(db,sql,select_callback,(void*)select_data,&zErrMsg);

    // get user's crypto balance
    sprintf(sql,"SELECT crypto_balance FROM Cryptos WHERE user_id = %d AND crypto_name = '%s';",user_id,crypto_name);
    sql_execute = sqlite3_exec(db,sql,select_callback,(void*)select_data,&zErrMsg);

    // if user has enough usd to buy crypto, subtract crypto amount from user's usd balance
    if (user_usd_balance >= crypto_amount * crypto_price) {
        sprintf(sql,"UPDATE Users SET usd_balance = usd_balance - %f WHERE ID = %d;",crypto_amount * crypto_price,user_id);
        sql_execute = sqlite3_exec(db,sql,update_callback,(void*)update_data,&zErrMsg);
    } else {
        fprintf(stderr,"Not enough USD to buy crypto.\n");
    }
    // if user already has crypto, add crypto amount to user's crypto balance
    if (user_crypto_balance > 0) {
        sprintf(sql,"UPDATE Cryptos SET crypto_balance = crypto_balance + %f WHERE user_id = %d AND crypto_name = '%s';",crypto_amount,user_id,crypto_name);
        sql_execute = sqlite3_exec(db,sql,update_callback,(void*)update_data,&zErrMsg);
    } else {
        // if user doesn't have crypto, add crypto to user's crypto balance
        sprintf(sql,"INSERT INTO Cryptos (crypto_name,crypto_balance,user_id) VALUES ('%s',%f,%d);",crypto_name,crypto_amount,user_id);
        sql_execute = sqlite3_exec(db,sql,insert_callback,0,&zErrMsg);
    }
    check_entry(sql_execute,zErrMsg);
}

void sell_crypto(sqlite3 *db, char *zErrMsg, int sql_execute, char *sql, const char *crypto_name, double crypto_amount,\
                double crypto_price, int user_id) {
    const char* select_data = "SELECT OPERATION";
    const char* update_data = "UPDATE OPERATION";
    double user_crypto_balance = 0.0;

    // get user's crypto balance
    sprintf(sql,"SELECT crypto_balance FROM Cryptos WHERE user_id = %d AND crypto_name = '%s';",user_id,crypto_name);
    sql_execute = sqlite3_exec(db,sql,select_callback,(void*)select_data,&zErrMsg);

    // if user has enough crypto to sell, subtract crypto amount from user's crypto balance
    if (crypto_amount <= user_crypto_balance) {
        sprintf(sql,"UPDATE Cryptos SET crypto_balance = crypto_balance - %f WHERE user_id = %d AND crypto_name = '%s';",crypto_amount,user_id,crypto_name);
        sql_execute = sqlite3_exec(db,sql,update_callback,(void*)update_data,&zErrMsg);

        // add crypto amount to user's usd balance
        sprintf(sql,"UPDATE Users SET usd_balance = usd_balance + %f WHERE ID = %d;",crypto_amount * crypto_price,user_id);
        sql_execute = sqlite3_exec(db,sql,update_callback,(void*)update_data,&zErrMsg);

        check_entry(sql_execute,zErrMsg);
    } else {
        printf("Not enough crypto to sell.\n");
    }
    check_entry(sql_execute,zErrMsg);
}

void list_all(sqlite3 *db, char *zErrMsg, int sql_execute, char *sql) {
  // list all records in the crypto database for user with user_id 1
  const char* data = "SELECT OPERATION";
  sprintf(sql,"SELECT * FROM Cryptos WHERE user_id = 1;");
  sql_execute = sqlite3_exec(db,sql,select_callback,(void*)data,&zErrMsg);
  check_entry(sql_execute,zErrMsg);
}


void close_database(sqlite3 *db)
{
    sqlite3_close(db);
}


