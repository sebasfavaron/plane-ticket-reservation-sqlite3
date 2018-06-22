#include <stdio.h>
#include <sqlite3.h>

int main(int argc, char **argv){
    sqlite3 *airportDB;
   char *errorMessage = 0;
   int rc;

   rc = sqlite3_open("airport.db", &airportDB);

   if(rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(airportDB));
      return 0;
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

    

   sqlite3_close(airportDB);
}