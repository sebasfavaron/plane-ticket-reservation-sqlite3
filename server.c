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

#define PUERTO "6667"
#define BACKLOG 5           // Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024    // Define cual va a ser el size maximo del paquete a enviar

//gcc server.c -pthread -l sqlite3

//Only one table called flights contains all the airport data
//create table flights (flightName string, seatNumber int, isTaken int, primary key (flightName,seatNumber));

char *TABLENAME = "flights";
sqlite3 *airportDB;

// Called once for every row returned by a sql query
static int tableExists(void *boolean, int columnCount, char **data, char **columns) {
   int *alreadyExists = boolean;
   //printf("tableExists\nBoolean before: %d\n",*alreadyExists);
   for(int i=0; i<columnCount; i++){
       //printf("%s = %s\n",columns[i],data[i]);
   }
   if(strcmp(data[0], "0")) *alreadyExists = 1;
   //printf("Count result: %s\nBoolean after: %d\n\n",data[0],*alreadyExists);
   return 0;
}

int createFlight(int seatAmount, char *flightName){
    int isTaken = 0, rc, alreadyExists = 0;
    char *errorMessage;    
    char existsSQL[100];
    
    snprintf(existsSQL,sizeof(existsSQL),"select count(flightName) from %s where flightName=\"%s\"",TABLENAME,flightName);
    rc = sqlite3_exec(airportDB, existsSQL, tableExists, (void*)&alreadyExists, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if(alreadyExists) {
        printf("Flight %s already exists\n\n", flightName);
        return -1;
    }

    for(int currSeat=0; currSeat<seatAmount; currSeat++){
        char sql[100];
        snprintf(sql, sizeof(sql), "insert into %s values(\"%s\",%d,%d)", TABLENAME, flightName, currSeat+1, isTaken);
        //printf("Command: %s\n",sql);
        rc = sqlite3_exec(airportDB, sql, NULL, 0, &errorMessage);
        if(rc != SQLITE_OK){
            fprintf(stderr, "SQL insert error: %s\n\n", errorMessage);
            sqlite3_free(errorMessage);
        } else {
            printf("Value %i inserted correctly into table %s, in flight %s\n", currSeat, TABLENAME, flightName);
        }
    }
    return 0;
}

int cancelFlight(char *flightName){
    int rc;
    char deleteSQL[100];
    char *errorMessage;
    
    snprintf(deleteSQL,sizeof(deleteSQL),"delete from %s where flightName=\"%s\"",TABLENAME, flightName);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL delete error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Flight %s deleted correctly\n\n", flightName);
        return 0;
    }
}

// No creo que se use nunca
int clearTable(){
    int rc;
    char deleteSQL[100];
    char *errorMessage;
    
    snprintf(deleteSQL,sizeof(deleteSQL),"delete from %s",TABLENAME);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL delete error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Table %s deleted correctly\n\n", TABLENAME);
        return 0;
    }
}

// funcion para debugging, pero no anda
int customQuery(){
    int rc;
    char customSQL[100];
    char *errorMessage;
    
    scanf("%s",customSQL);
    rc = sqlite3_exec(airportDB, customSQL, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL custom query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Custom query correctly executed\n\n");
        return 0;
    }
}

static int seatTaken(void *boolean, int columnCount, char **data, char **columns) {
   int *taken = boolean;
   //printf("seatTaken\nBoolean before: %c\n",*taken);
   for(int i=0; i<columnCount; i++){
       printf("%s = %s\n",columns[i],data[i]);
   }
   if(strcmp(data[0], "0")) *taken = 1;
   //printf("Count result: %s\nBoolean after: %d\n\n",data[0],*taken);
   return 0;
}

int reserveSeat(char *flightName, int seatNumber){
    int rc, isTaken = 0;
    char takenQuery[100];
    char *errorMessage;
    
    snprintf(takenQuery,sizeof(takenQuery),"select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, takenQuery, seatTaken, (void*)&isTaken, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if(isTaken) {
        printf("Seat %d in flight %s already taken\n\n", seatNumber, flightName);
        return -1;
    }

    char query[100];
    snprintf(query, sizeof(query), "update %s set isTaken=1 where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Seat %d in flight %s correctly reserved\n\n", seatNumber, flightName);
        return 0;
    }
}

int cancelSeat(char *flightName, int seatNumber){
    int rc, isTaken = 1;
    char cancelQuery[100];
    char *errorMessage;
    
    snprintf(cancelQuery,sizeof(cancelQuery),"select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, cancelQuery, seatTaken, (void*)&isTaken, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if(!isTaken) {
        printf("Seat %d in flight %s is not taken, so it can't be canceled\n\n", seatNumber, flightName);
        return -1;
    }

    char query[100];
    snprintf(query, sizeof(query), "update %s set isTaken=0 where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Seat %d in flight %s correctly canceled\n\n", seatNumber, flightName);
        return 0;
    }
}

void closeDatabase(){
    sqlite3_close(airportDB);
    printf("Closed database\n");
}

int main(int argc, char **argv){
    char *errorMessage;

    int fd,fd2,longitud_cliente,puerto;

    puerto=PUERTO;

    struct sockaddr_in server;
    struct sockaddr_in client;

    //Configuracion del servidor
    server.sin_family= AF_INET; //Familia TCP/IP
    server.sin_port = htons(puerto); //Puerto
    server.sin_addr.s_addr = INADDR_ANY; //Cualquier cliente puede conectarse
    bzero(&(server.sin_zero),8); //Funcion que rellena con 0's

    if (( fd=socket(AF_INET,SOCK_STREAM,0) )<0)
    {
        perror("Error definiendo el socket");
        exit(-1);
    }

    if(bind(fd,(struct sockaddr*)&server, sizeof(struct sockaddr))==-1) 
    {
        printf("error en bind() \n");
        exit(-1);
    }

    if(listen(fd,5) == -1) 
    {
    printf("error en listen()\n");
    exit(-1);
    }

    //acepto conexiones
    while(1) 
    {
    longitud_cliente = sizeof(struct sockaddr_in);

    if ((fd2 = accept(fd,(struct sockaddr *)&client,&longitud_cliente))==-1) 
    {
    printf("error en accept()\n");
    exit(-1);
    }

    // Aca el servidor envia el mensaje que queramos.
    //El 2do parametro es el mensaje y el 3ro la longitud.
    send(fd2,"Bienvenido a mi servidor.\n",26,0);
    close(fd2); /* cierra fd2 */
    }

    close(fd);

    int rc = sqlite3_open("airport.db", &airportDB);
    if(rc) {
        fprintf(stderr, "\nCan't open database: %s\n\n", sqlite3_errmsg(airportDB));
        return 0;
    } else {
        fprintf(stderr, "\nOpened database successfully\n\n");
    }
    //crear tabla flights si no existe
    char createFlights[256] = "create table if not exists flights (flightName string, seatNumber int, isTaken int, primary key (flightName,seatNumber))";
    rc = sqlite3_exec(airportDB, createFlights, NULL, NULL, NULL);
    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL query error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    } else {
        printf("Table \"flights\" correctly initialized/created\n\n");
    }

    //aca atiende a los clientes con forks y threads. Muchas funciones de sql creadas aca
    //van a tener que moverse a un requestHandler.c a donde mandemos a cada cliente
    
    //si los querys estan bien no tira error, pero no avisa que esta mal

    closeDatabase();

    return 0;
}