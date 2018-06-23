#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sqlite3.h>
#include <netinet/in.h>

#define TABLENAME "flights"

static int tableExists(void *boolean, int columnCount, char **data, char **columns)
int createFlight(int seatAmount, char *flightName)
int cancelFlight(char *flightName)
int clearTable()
int customQuery()
static int seatTaken(void *boolean, int columnCount, char **data, char **columns)
int reserveSeat(char *flightName, int seatNumber)
int cancelSeat(char *flightName, int seatNumber)
void closeDatabase()


#endif