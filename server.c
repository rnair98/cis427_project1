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
  int buf_len, addr_len;
  int s, new_s;
  char UnCmdMsg[MAX_LINE] = "400 invalid command\n";
  char ShutMsg [MAX_LINE] = "200 OK\n";
  char ForMsg [MAX_LINE] = "403 message format error\n";

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



  /* wait for connection, then receive user input and print response */
  while(1) {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
      perror("crypto-trading client: accept");
      exit(1);
    }
    while ((buff_len = recv(new_s, buf, sizeof(buf), 0))) {
      printf("Received: %s",buf);
      char * cmd = strtok(buf, " ");

      if ( strcmp(cmd, "SHUTDOWN\n")==0 ) {
        send(new_s, ShutMsg, sizeof(ShutMsg), 0);
	close_database(db);
	close(new_s);
	return 0;
      }
      else if ( strcmp(cmd, "LIST\n")==0 ) {
        // list
      }

