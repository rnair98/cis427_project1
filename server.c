#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server_functions.h"

int main(int argc, char* argv[]) {
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    unsigned int buf_len, addr_len;
    int s, new_s;
    char UnCmdMsg[MAX_LINE] = "400 invalid command\n";
    char ShutMsg [MAX_LINE] = "200 OK\n";
    char ForMsg [MAX_LINE] = "403 message format error\n";

    // database variables
    sqlite3* db;
    char *zErrMsg = 0;
    int sql_execute = 0;
    char *sql;
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
	            close_database(db);
                close(new_s);
                exit(0);
            } else if (strcmp(cmd, "QUIT\n")) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
            }
            else if ( strcmp(cmd, "LIST\n")==0 ) {
                send(new_s, ShutMsg, sizeof(ShutMsg), 0);
                list_all(db,zErrMsg,sql_execute,sql);
            } else if ( strcmp(cmd, "SELL")==0 ) {
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



