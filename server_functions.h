#ifndef SERVER_FUNCTIONS
#define SERVER_FUNCTIONS

#define SERVER_PORT 9267
#define MAX_PENDING 5
#define MAX_LINE    256

#include <sqlite3.h>

int insert_callback(void *NotUsed, int argc, char **argv, char **azColName);
int select_callback(void *data, int argc, char **argv, char **azColName);
int update_callback(void *data, int argc, char **argv, char **azColName);

int init_database(sqlite3 *db, char *zErrMsg,int sql_execute,char *sql); 
void close_database(sqlite3 *db);
void show_balance(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql, int user_id);
void buy_crypto(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,\
                const char* crypto_name,double crypto_amount,double crypto_price,int user_id);
void sell_crypto(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,\
                 const char* crypto_name,double crypto_amount,double crypto_price,int user_id);
void list_all(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql);

#endif
