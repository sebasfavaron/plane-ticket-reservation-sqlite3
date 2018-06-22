#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sqlite3.h>

char *TABLENAME = "flights";
sqlite3 *airportDB;
int createFlight(int seatAmount, char *flightName){
    
    char isTaken = '0';
    char *errorMessage;
    int rc;

    for(int currSeat=0; currSeat<seatAmount; currSeat++){
        char *sql = "insert into ";
        strcat(sql, TABLENAME);
        strcat(sql, "values(");
        strcat(sql, flightName);
        strcat(sql, ",");
        char seat[10];
        itoa(currSeat, seat, 10);
        strcat(sql, seat);
        strcat(sql, ",");
        strcat(sql, isTaken);
        strcat(sql, ");");
        rc = sqlite3_exec(airportDB, sql, NULL, 0, &errorMessage);
        if(rc != SQLITE_OK){
            fprintf(stderr, "SQL insert error: %s\n", errorMessage);
            sqlite3_free(errorMessage);
        } else {
            printf("Value %i inserted correctly into table %s", currSeat, flightName);
        }
    }
}

int main(int argc, char **argv){
   int rc = sqlite3_open("airport.db", &airportDB);

   if(rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(airportDB));
      return 0;
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

    createFlight(4,"ACB132");

   sqlite3_close(airportDB);
}