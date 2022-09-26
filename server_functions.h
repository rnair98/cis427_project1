#ifndef SERVER_FUNCTIONS
#define SERVER_FUNCTIONS

#define SERVER_PORT 9267
#define MAX_PENDING 5
#define MAX_LINE    256

#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName);

void init_database(sqlite3 *db, char *zErrMsg, int rc; char *sql); 

#endif
