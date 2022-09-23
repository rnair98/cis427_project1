#include <iostream>
#include <sqlite3.h>

using namespace std;

int main(){
  sqlite3 db;
  char ErrMsg = 0;
  int rc;

  rc = sqlite3_open("./cis427_crypto.sqlite", &db);

  if (rc) {
   cout << "Can't open database.\n";
   return 0;
  } else {
   cout << "Opened database successfully\n";
  }
  sqlite3_close(db);
}
