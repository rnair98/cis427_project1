#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 9267
#define MAX_LINE 256


int main(int argc, char * argv[]) {
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int len;
  const char * commands[] = {
    "LIST\n",
    "BALANCE\n",
    "SHUTDOWN\n",
    "QUIT\n",
  };
  char *search;

  if (argc==2) {
    host = argv[1];
  } else {
    fprintf(stderr, "usage: crypto-trading client host\n");
    exit(1);
  }

  /* translate hostname into peer's IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr,"crypto-trading client: unknown host: %s\n",host);
    exit(1);
  }

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

  /* active open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("crypto-trading client: socket");
    exit(1);
  }
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("crypto-trading client: connect");
    exit(1);
  }

  /* main loop - get and send lines of text */
  while (fgets(buf, sizeof(buf), stdin)) {
    buf[MAX_LINE-1] = '\0';
    len = strlen(buf) + 1;
    search = strstr(commands,buf);
    if (search != NULL) {
      if (strcmp(buf,"QUIT\n")==0){
        printf("200 OK\n");
	break;
      } else {
        send(s,buf,len,0);
      }
    } 
    else if (strstr(buf,"BUY")) {
      send(s,buf,len,0);
    } 
    else if (strstr(buf,"SELL")) {
      send(s,buf,len,0);
    } 
    else {
      printf("400 invalid command");
      break;
    }
  }
}
