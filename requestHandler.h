#ifndef CLIENT
#define CLIENT

#define TABLENAME "flights"

static int tableExists(void *boolean, int columnCount, char **data, char **columns);
int createFlight(int seatAmount, char *flightName);
int cancelFlight(char *flightName);
int clearTable();
int customQuery();
static int seatTaken(void *boolean, int columnCount, char **data, char **columns);
int reserveSeat(int seatNumber, char *flightName);
int cancelSeat(int seatNumber, char *flightName);

#endif