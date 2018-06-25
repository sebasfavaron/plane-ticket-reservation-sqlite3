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
#include "server.h"

sqlite3 *airportDB;

static int tableExists(void *boolean, int columnCount, char **data, char **columns)
{
    int *alreadyExists = boolean;
    //printf("tableExists\nBoolean before: %d\n",*alreadyExists);
    for (int i = 0; i < columnCount; i++)
    {
        //printf("%s = %s\n",columns[i],data[i]);
    }
    if (strcmp(data[0], "0"))
        *alreadyExists = 1;
    //printf("Count result: %s\nBoolean after: %d\n\n",data[0],*alreadyExists);
    return 0;
}

int createFlight(int seatAmount, char *flightName)
{
    int isTaken = 0, rc, alreadyExists = 0;
    char *errorMessage;
    char existsSQL[100];

    snprintf(existsSQL, sizeof(existsSQL), "select count(flightName) from %s where flightName=\"%s\"", TABLENAME, flightName);
    rc = sqlite3_exec(airportDB, existsSQL, tableExists, (void *)&alreadyExists, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if (alreadyExists)
    {
        printf("Flight %s already exists\n\n", flightName);
        return -1;
    }

    for (int currSeat = 0; currSeat < seatAmount; currSeat++)
    {
        char sql[100];
        snprintf(sql, sizeof(sql), "insert into %s values(\"%s\",%d,%d,\"%s\")", TABLENAME, flightName, currSeat + 1, isTaken, "NULL");
        //printf("Command: %s\n",sql);
        rc = sqlite3_exec(airportDB, sql, NULL, 0, &errorMessage);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL insert error: %s\n\n", errorMessage);
            sqlite3_free(errorMessage);
        }
    }
    printf("Flight %s created in table %s with %d seats\n", flightName, TABLENAME, seatAmount);
    return 0;
}

int cancelFlight(char *flightName)
{
    int rc;
    char deleteSQL[100];
    char *errorMessage;

    snprintf(deleteSQL, sizeof(deleteSQL), "delete from %s where flightName=\"%s\"", TABLENAME, flightName);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL delete error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else
    {
        printf("Flight %s deleted correctly\n\n", flightName);
        return 0;
    }
}

static int printSeat(void *seats, int columnCount, char **data, char **columns)
{
    char **seatsString = (char **)seats;
    char *isTaken = data[2];
    
    if(sizeof(*seatsString) % 10 == 0)
        *seatsString = realloc(*seatsString, sizeof(*seatsString) + 10);
    strcat(*seatsString, isTaken);

    return 0;
}

char *makeFlightString(char *seats){
    int planeWidth = 10, seatsSize = sizeof(seats);
    char *flight = malloc(seatsSize*2 + (int)(seatsSize/10*15));
    strcpy(flight,"0  | ");
    for(int i=0; i<seatsSize; i++){
        sprintf(flight + strlen(flight), "&c ", seats[i]);
        /* char seat[2];
        sprintf(seat, "&c ", seats[i]);
        strcat(flight, seat); */
        if(i % planeWidth == 0) {
            sprintf(flight + strlen(flight), "|  %d\n%d  | ", i, i+1);
            /* char extra[10];
            strcpy(extra, "|  %d\n%d  |", i, i+1);
            strcat(flight, extra); */
        }
    }
    return flight;
}

char *showFlight(char *flightName)
{
    int rc;
    char printSQL[100], *errorMessage, *seats;
    seats = (char *) malloc(10 * sizeof(char));
    snprintf(printSQL, sizeof(printSQL), "select from %s where flightName=\"%s\"", TABLENAME, flightName);
    rc = sqlite3_exec(airportDB, printSQL, printSeat, (void *)&seats, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Flight print error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return NULL;
    }
    else
    {
        return makeFlightString(seats);
    }
}

// No creo que se use nunca
int clearTable()
{
    int rc;
    char deleteSQL[100];
    char *errorMessage;

    snprintf(deleteSQL, sizeof(deleteSQL), "delete from %s", TABLENAME);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL delete error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else
    {
        printf("Table %s deleted correctly\n\n", TABLENAME);
        return 0;
    }
}

// funcion para debugging, pero no anda
int customQuery()
{
    int rc;
    char customSQL[100];
    char *errorMessage;

    scanf("%s", customSQL);
    rc = sqlite3_exec(airportDB, customSQL, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL custom query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else
    {
        printf("Custom query correctly executed\n\n");
        return 0;
    }
}

static int seatTaken(void *boolean, int columnCount, char **data, char **columns)
{
    int *taken = boolean;

    if (strcmp(data[0], "0") == 0)
        *taken = 0;
    else
        *taken = 1;

    return 0;
}

int reserveSeat(int seatNumber, char *flightName, char *client)
{
    int rc, isTaken = 1;
    char takenQuery[256];
    char *errorMessage;

    snprintf(takenQuery, sizeof(takenQuery), "select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, takenQuery, seatTaken, (void *)&isTaken, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if (isTaken)
    {
        printf("Seat %d in flight %s already taken or does not exist\n\n", seatNumber, flightName);
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "update %s set isTaken=1 where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    snprintf(query, sizeof(query), "update %s set clientName=%s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, client, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else
    {
        printf("Seat %d in flight %s correctly reserved\n\n", seatNumber, flightName);
        return 0;
    }
}

static int executionCounter(void *i, int columnCount, char **data, char **columns)
{
    int *counter = i;

    (*counter)++;

    return 0;
}

int cancelSeat(int seatNumber, char *flightName, char *client)
{
    int rc, isTaken = 0, i = 0;
    char cancelQuery[256];
    char *errorMessage;

    snprintf(cancelQuery, sizeof(cancelQuery), "select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, cancelQuery, seatTaken, (void *)&isTaken, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if (!isTaken)
    {
        printf("Seat %d in flight %s is not taken or does not exist, so it can't be canceled\n\n", seatNumber, flightName);
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "update %s set isTaken=0 where flightName=\"%s\" and seatNumber=\"%d\" and clientName=\"%s\"", TABLENAME, flightName, seatNumber, client);
    rc = sqlite3_exec(airportDB, query, executionCounter, (void *)&i, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else if (i == 0)
    {
        printf("That seat doesn't belong to you\n");
    }
    else
    {
        printf("Seat %d in flight %s correctly canceled\n\n", seatNumber, flightName);
        return 0;
    }
}

int stringToInt(char *string)
{
    int i, len, result = 0;

    len = strlen(string);
    for (i = 0; i < len; i++)
    {
        if (string[i] == '\n' || string[i] == ' ')
            break;
        if (string[i] < '0' || string[i] > '9')
            return -1;
        result = result * 10 + (string[i] - '0');
    }

    return result;
}

int parseMessage(char *buf, char **command, char **nameFlight, int *seat, char **client)
{
    int i = 0;
    //1er if: command reste 1,
    if (strncmp(buf, "create flight", 13) == 0)
    {
        strcpy(*command, "create flight");
        i += 14;
        int nameLength = strstr(buf + i, " ") - (buf + i);
        strncpy(*nameFlight, buf + i, nameLength);
        i += nameLength + 1;
        *seat = stringToInt(buf + i);
    }
    else if (strncmp(buf, "cancel flight", 13) == 0)
    {
        strcpy(*command, "cancel flight");
        i += 14;
        strcpy(*nameFlight, buf + i);
        (*nameFlight)[strlen(*nameFlight) - 1] = 0;
    }
    else if (strncmp(buf, "book", 4) == 0)
    {
        strcpy(*command, "book");
        i += 5;
        int nameLength = strstr(buf + i, " ") - (buf + i);
        strncpy(*nameFlight, buf + i, nameLength);
        i += nameLength + 1;
        /* *seat = stringToInt(buf + i);
        i += strstr(buf + i, " ") - (buf + i); */
        strcpy(*client, buf + i);
        (*client)[strlen(*client) - 1] = 0;
    }
    else if (strncmp(buf, "cancel seat", 11) == 0)
    {
        strcpy(*command, "cancel seat");
        i += 12;
        int nameLength = strstr(buf + i, " ") - (buf + i);
        strncpy(*nameFlight, buf + i, nameLength);
        i += nameLength + 1;
        /* *seat = stringToInt(buf + i);
        i += strstr(buf + i, " ") - (buf + i); */
        strcpy(*client, buf + i);
        (*client)[strlen(*client) - 1] = 0;
    }
    else
    {
        printf("Wrong command\n");
        return -1;
    }
    return 0;
}

int requestHandler(char *command, char *nameFlight, int seat, char *client, int fd2)
{
    if (strcmp(command, "create flight") == 0)
    {
        printf("%s", showFlight("AA323"));
        createFlight(seat, nameFlight);
    }
    else if (strcmp(command, "cancel flight") == 0)
    {
        cancelFlight(nameFlight);
    }
    else if (strcmp(command, "book") == 0)
    {
        // char *msgToClient = showFlight(nameFlight);
        char *msgToClient;
        strcpy(msgToClient,"string"); 

        // Aca el servidor envia el mensaje que queramos.
        //El 2do parametro es el mensaje y el 3ro la longitud.
        send(fd2, msgToClient, strlen(msgToClient)+1, 0);
        //aca hay que mandarle msgToClient y averiguar el seat que quiere

        reserveSeat(seat, nameFlight, client);
    }
    else if (strcmp(command, "cancel seat") == 0)
    {
        char *msgToClient = showFlight(nameFlight);

        //aca hay que mandarle msgToClient y averiguar el seat que quiere

        cancelSeat(seat, nameFlight, client);
    }
    printf("end of requestHandler");
    return 0;
}

int openDatabase()
{
    int rc = sqlite3_open("airport.db", &airportDB);
    if (rc)
    {
        fprintf(stderr, "\nCan't open database: %s\n", sqlite3_errmsg(airportDB));
        return -1;
    }
    else
    {
        fprintf(stderr, "\nOpened database successfully\n");
    }
    return 0;
}

int closeDatabase()
{
    int rc = sqlite3_close(airportDB);
    if (rc)
    {
        fprintf(stderr, "\nCan't close database: %s\n\n", sqlite3_errmsg(airportDB));
        return 0;
    }
    else
    {
        fprintf(stderr, "\nClosed database successfully\n");
    }
}

int main(int argc, char **argv)
{
    char *errorMessage;

    int fd, fd2, longitud_cliente, puerto, rc;

    //Abro/creo airport.db
    if (openDatabase() != 0)
        return -1;

    //Crea tabla flights si no existe
    char createFlights[256] = "create table if not exists flights (flightName string, seatNumber int, isTaken int, clientName string, primary key (flightName,seatNumber))";
    rc = sqlite3_exec(airportDB, createFlights, NULL, NULL, &errorMessage);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error initializing/creating table \"flights\": %s\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    else
    {
        printf("Table \"flights\" correctly initialized/created\n\n");
    }

    // puerto = PUERTO;

    struct sockaddr_in server;
    struct sockaddr_in client;

    //Configuracion del servidor
    server.sin_family = AF_INET;         //Familia TCP/IP
    server.sin_port = htons(PUERTO);     //Puerto
    server.sin_addr.s_addr = INADDR_ANY; //Cualquier cliente puede conectarse
    bzero(&(server.sin_zero), 8);        //Funcion que rellena con 0's

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error definiendo el socket");
        exit(-1);
    }

    int yes=1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
    {
    perror("setsockopt");
    exit(1);
    }

    if (bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("error en bind() \n");
        close(fd);
        exit(-1);
    }

    if (listen(fd, 5) == -1)
    {
        printf("error en listen()\n");
        exit(-1);
    }

    //acepto conexiones
    while (1)
    {
        longitud_cliente = sizeof(struct sockaddr_in);

        if ((fd2 = accept(fd, (struct sockaddr *)&client, &longitud_cliente)) == -1)
        {
            printf("error en accept()\n");
            exit(-1);
        }

        //aca recibe un mensaje
        char buf[1024]; //Para recibir mensaje
        recv(fd2, buf, 1024, 0);

        char *command, *nameFlight, *client;
        int seat = 0;
        command = (char *)malloc(50 * sizeof(char));
        nameFlight = (char *)malloc(50 * sizeof(char));

        if (fork() == 0)
        {
            rc = parseMessage(buf, &command, &nameFlight, &seat, &client);

            if (seat < 0 || rc != 0)
                return -1;

            requestHandler(command, nameFlight, seat, client, fd2);

            // return 0;
        }

        close(fd2); /* cierra fd2 */
    }

    close(fd);

    if (closeDatabase() != 0)
        return -1;

    return 0;
}