#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server_functions.h"


//global variables
int s, new_s;
char buf[MAX_LINE];

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      sprintf(buf,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      send(new_s, buf, strlen(buf), 0);
   }
   sprintf(buf,("\n"));
   send(new_s, buf, strlen(buf), 0);
   return 0;
}

int insert_callback(void *NotUsed, int argc, char ** argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++) {
        sprintf(buf,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        send(new_s,buf,strlen(buf),0);
    }
    sprintf(buf  ,"\n");
    return 0;
}

int select_callback(void *data, int argc, char **argv, char **azColName){
   int i;
   for(i = 0; i<argc; i++){
      sprintf(buf,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      send(new_s,buf,strlen(buf),0);
   }
   return 0;
}

int update_callback(void *data, int argc, char **argv, char **azColName){
   int i;
   char buf[MAX_LINE]="";

   for(i = 0; i<argc; i++) {
      sprintf(buf,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      send(new_s,buf,strlen(buf),0);
   }
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

        sql_execute = sqlite3_exec(db,sql,callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);

        /* Create CRYPTOS Table */

        sql = "CREATE TABLE if NOT EXISTS Cryptos("\
	            "ID int NOT NULL,"\
	            "crypto_name varchar(10) NOT NULL,"\
	            "crypto_balance DOUBLE,"\
	            "user_id int,"\
	            "PRIMARY KEY (ID),"\
	            "FOREIGN KEY (user_id) REFERENCES Users (ID));";
        sql_execute = sqlite3_exec(db,sql,callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);

        /* if USERS doesn't contain 1 person, add a sample user */
  
        sql = "INSERT OR IGNORE INTO Users("\
	            "ID, first_name, last_name, user_name,"\
	            "password, usd_balance, email) "\
	            "VALUES (1,'John','Doe','jdoe','password123',"\
	            "100.00,'jdoe@umich.edu');";
  
        sql_execute = sqlite3_exec(db,sql,callback,0,&zErrMsg);
        check_entry(sql_execute,zErrMsg);
        return open_database;
    }
}

void show_balance(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,int user_id) {
  const char* data = "SELECT OPERATION";
  sprintf(sql,"SELECT usd_balance FROM users WHERE ID = %d;",user_id);
  sql_execute = sqlite3_exec(db,sql,callback,(void*)data,&zErrMsg);
  check_entry(sql_execute,zErrMsg);
}

void buy_crypto(sqlite3 *db,char *zErrMsg,int sql_execute,char *sql,const char* crypto_name,double crypto_amount\
                ,double crypto_price, int user_id) {
    const char* select_data = "SELECT OPERATION";
    const char* update_data = "UPDATE OPERATION";
    double user_usd_balance = 0.0;
    double user_crypto_balance = 0.0;

    // get user's usd balance
    sprintf(sql,"SELECT usd_balance FROM users WHERE ID = %d;",user_id);
    sql_execute = sqlite3_exec(db,sql,callback,(void*)select_data,&zErrMsg);

    // get user's crypto balance
    sprintf(sql,"SELECT crypto_balance FROM cryptos WHERE user_id = %d AND crypto_name = '%s';",user_id,crypto_name);
    sql_execute = sqlite3_exec(db,sql,callback,(void*)select_data,&zErrMsg);

    // if user has enough usd to buy crypto, subtract crypto amount from user's usd balance
    if (user_usd_balance >= crypto_amount * crypto_price) {
        sprintf(sql,"UPDATE users SET usd_balance = usd_balance - %f WHERE ID = %d;",crypto_amount * crypto_price,user_id);
        sql_execute = sqlite3_exec(db,sql,callback,(void*)update_data,&zErrMsg);
    } else {
        fprintf(stderr,"Not enough USD to buy crypto.\n");
    }
    // if user already has crypto, add crypto amount to user's crypto balance
    if (user_crypto_balance > 0) {
        sprintf(sql,"UPDATE cryptos SET crypto_balance = crypto_balance + %f WHERE user_id = %d AND crypto_name = '%s';",crypto_amount,user_id,crypto_name);
        sql_execute = sqlite3_exec(db,sql,callback,(void*)update_data,&zErrMsg);
    } else {
        // if user doesn't have crypto, add crypto to user's crypto balance
        sprintf(sql,"INSERT INTO cryptos (crypto_name,crypto_balance,user_id) VALUES ('%s',%f,%d);",crypto_name,crypto_amount,user_id);
        sql_execute = sqlite3_exec(db,sql,callback,0,&zErrMsg);
    }
    check_entry(sql_execute,zErrMsg);
}

void sell_crypto(sqlite3 *db, char *zErrMsg, int sql_execute, char *sql, const char *crypto_name, double crypto_amount,\
                double crypto_price, int user_id) {
    const char* select_data = "SELECT OPERATION";
    const char* update_data = "UPDATE OPERATION";
    double user_crypto_balance = 0.0;

    // get user's crypto balance
    sprintf(sql,"SELECT crypto_balance FROM cryptos WHERE user_id = %d AND crypto_name = '%s';",user_id,crypto_name);
    sql_execute = sqlite3_exec(db,sql,callback,(void*)select_data,&zErrMsg);

    // if user has enough crypto to sell, subtract crypto amount from user's crypto balance
    if (crypto_amount <= user_crypto_balance) {
        sprintf(sql,"UPDATE cryptos SET crypto_balance = crypto_balance - %f WHERE user_id = %d AND crypto_name = '%s';",crypto_amount,user_id,crypto_name);
        sql_execute = sqlite3_exec(db,sql,callback,(void*)update_data,&zErrMsg);

        // add crypto amount to user's usd balance
        sprintf(sql,"UPDATE users SET usd_balance = usd_balance + %f WHERE ID = %d;",crypto_amount * crypto_price,user_id);
        sql_execute = sqlite3_exec(db,sql,callback,(void*)update_data,&zErrMsg);

        check_entry(sql_execute,zErrMsg);
    } else {
        printf("Not enough crypto to sell.\n");
    }
    check_entry(sql_execute,zErrMsg);
}

void list_all(sqlite3 *db, char *zErrMsg, int sql_execute, char *sql) {
  // list all records in the crypto database for user with user_id 1
  const char* data = "";
  sprintf(sql,"SELECT * FROM users WHERE ID = 1;");
  sql_execute = sqlite3_exec(db,sql,callback,(void*)data,&zErrMsg);
  check_entry(sql_execute,zErrMsg);
}

void close_database(sqlite3 *db)
{
    sqlite3_close(db);
}

//function to check if input string is alphanumeric
int isAlphaNumeric(char *str)
{
    while (*str != '\0')
    {
        if (isalnum(*str++) == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    struct sockaddr_in sin;
    unsigned int buf_len, addr_len;
    char endM[MAX_LINE] = "<>";
    char UnCmdMsg[MAX_LINE] = "400 invalid command\n";
    char ShutMsg [MAX_LINE] = "200 OK\n";
    char ForMsg [MAX_LINE] = "403 message format error\n";

    // database variables
    sqlite3* db = NULL;
    char *zErrMsg = 0;
    int sql_execute = 0;
    char *sql = NULL;
    const char* crypto_name;
    double crypto_amount = 0;
    double crypto_price_in_usd = 0;
    int user_id = 0;
    int command_count = 0;

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("crypto-trading client: socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("crypto-trading client: bind");
        exit(1);
    }
    listen(s, MAX_PENDING);

    /* initialize crypto database */

    init_database(db,zErrMsg, sql_execute, sql);

    /* wait for connection, then receive user input and print response */
    while(1) {
        if ((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
	    perror("crypto-trading client: accept");
            exit(1);
        }
        while ((buf_len = recv(new_s, buf, sizeof(buf), 0))) {
            printf("Received: %s",buf);
            char * cmd = strtok(buf, " ");
            command_count++;

            if ( strcmp(cmd, "SHUTDOWN\n")==0 ) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                send(new_s, endM, sizeof(endM), 0);
	            close_database(db);
                close(new_s);
                exit(0);
            } else if (strcmp(cmd, "QUIT\n")) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                send(new_s, endM, sizeof(endM), 0);
            }
            else if ( strcmp(cmd, "LIST\n")==0 ) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                list_all(db,zErrMsg,sql_execute,sql);
                send(new_s, endM, sizeof(endM), 0);
            } else if ( strcmp(cmd, "SELL")==0 ) {
                while( cmd != NULL ) {
                    if (command_count > 5) {
                        send(new_s, ForMsg, sizeof(ForMsg), 0);
                        break;
                    } else if (command_count == 1){
                        continue;
                    } else if (isAlphaNumeric(cmd) && command_count==2) {
                        crypto_name = cmd;
                    } else if ( atoi(cmd) > 0 && command_count==3 ) {
                        sscanf(cmd, "%lf", &crypto_amount);
                    } else if ( atoi(cmd) > 0 && command_count==4 ) {
                        sscanf(cmd, "%lf", &crypto_price_in_usd);
                    } else if ( atoi(cmd) > 0 && command_count==5 ) {
                        sscanf(cmd, "%d", &user_id);
                    } else {
                        send(new_s, ForMsg, sizeof(ForMsg), 0);
                        break;
                    }
                    cmd = strtok(NULL, " ");
                    command_count++;
                }
                if (command_count == 5 && crypto_name != NULL && crypto_amount > 0 && crypto_price_in_usd > 0 && user_id > 0) {
                    send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                    sell_crypto(db, zErrMsg, sql_execute, sql, crypto_name, crypto_amount, crypto_price_in_usd, user_id);
                }
            } else if ( strcmp(cmd, "BUY")==0) {
                while( cmd != NULL ) {
                       if (command_count > 5) {
                           send(new_s, ForMsg, sizeof(ForMsg), 0);
                           break;
                       } else if (command_count == 1){
                        continue;
                       } else if ( strcmp(cmd, "LEETCOIN")==0 && command_count==2) {
                        crypto_name = "LEETCOIN";
                       } else if ( atoi(cmd) > 0 && command_count==3 ) {
                        sscanf(cmd, "%lf", &crypto_amount);
                       } else if ( atoi(cmd) > 0 && command_count==4 ) {
                        sscanf(cmd, "%lf", &crypto_price_in_usd);
                       } else if ( atoi(cmd) > 0 && command_count==5 ) {
                        sscanf(cmd, "%d", &user_id);
                       } else {
                           send(new_s, ForMsg, sizeof(ForMsg), 0);
                           break;
                       }
                       cmd = strtok(NULL, " ");
                       command_count++;                       
                   }
                if (command_count == 5 && crypto_name != NULL && crypto_amount > 0 && crypto_price_in_usd > 0 && user_id > 0) {
                    send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                    buy_crypto(db, zErrMsg, sql_execute, sql, crypto_name, crypto_amount, crypto_price_in_usd, user_id);
                }
            } else if ( strcmp(cmd, "BALANCE\n")==0 ) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                show_balance(db,zErrMsg,sql_execute,sql,user_id);
            } else if ( strstr(cmd, "SHUTDOWN")) {
                send(new_s, ForMsg, sizeof(ForMsg), 0);
            } else if ( strstr(cmd, "LIST")) {
                send(new_s, ForMsg, sizeof(ForMsg), 0);
            } else if ( strstr(cmd, "SELL")) {
                send(new_s, ForMsg, sizeof(ForMsg), 0);
            } else if ( strstr(cmd, "BUY")) {
                send(new_s, ForMsg, sizeof(ForMsg), 0);
            } else if ( strstr(cmd, "BALANCE")) {
                send(new_s, ForMsg, sizeof(ForMsg), 0);
            } else {
                send(new_s, UnCmdMsg, sizeof(UnCmdMsg), 0);
            }
        }
        close(new_s);
    }
}