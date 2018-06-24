#ifndef SERVER
#define SERVER

int closeDatabase();
static int tableExists(void *boolean, int columnCount, char **data, char **columns);
int createFlight(int seatAmount, char *flightName);
int cancelFlight(char *flightName);
int clearTable();
int customQuery();
static int executionCounter(void *i, int columnCount, char **data, char **columns);
static int seatTaken(void *boolean, int columnCount, char **data, char **columns);
int reserveSeat(int seatNumber, char *flightName, char *client);
int cancelSeat(int seatNumber, char *flightName, char *client);

#define TABLENAME "flights"
#define PUERTO 6667
#define BACKLOG 5        // Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024 // Define cual va a ser el size maximo del paquete a enviar

#endif