#include <requestHandler.h>

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
    rc = sqlite3_exec(airportDB, existsSQL, tableExists, (void *)&alreadyExists, NULL);
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
        snprintf(sql, sizeof(sql), "insert into %s values(\"%s\",%d,%d)", TABLENAME, flightName, currSeat + 1, isTaken);
        //printf("Command: %s\n",sql);
        rc = sqlite3_exec(airportDB, sql, NULL, 0, &errorMessage);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL insert error: %s\n\n", errorMessage);
            sqlite3_free(errorMessage);
        }
        else
        {
            printf("Value %i inserted correctly into table %s, in flight %s\n", currSeat, TABLENAME, flightName);
        }
    }
    return 0;
}

int cancelFlight(char *flightName)
{
    int rc;
    char deleteSQL[100];
    char *errorMessage;

    snprintf(deleteSQL, sizeof(deleteSQL), "delete from %s where flightName=\"%s\"", TABLENAME, flightName);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, NULL);
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

// No creo que se use nunca
int clearTable()
{
    int rc;
    char deleteSQL[100];
    char *errorMessage;

    snprintf(deleteSQL, sizeof(deleteSQL), "delete from %s", TABLENAME);
    rc = sqlite3_exec(airportDB, deleteSQL, NULL, NULL, NULL);
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
    rc = sqlite3_exec(airportDB, customSQL, NULL, NULL, NULL);
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
    //printf("seatTaken\nBoolean before: %c\n",*taken);
    for (int i = 0; i < columnCount; i++)
    {
        printf("%s = %s\n", columns[i], data[i]);
    }
    if (strcmp(data[0], "0"))
        *taken = 1;
    //printf("Count result: %s\nBoolean after: %d\n\n",data[0],*taken);
    return 0;
}

int reserveSeat(int seatNumber, char *flightName)
{
    int rc, isTaken = 0;
    char takenQuery[100];
    char *errorMessage;

    snprintf(takenQuery, sizeof(takenQuery), "select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, takenQuery, seatTaken, (void *)&isTaken, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if (isTaken)
    {
        printf("Seat %d in flight %s already taken\n\n", seatNumber, flightName);
        return -1;
    }

    char query[100];
    snprintf(query, sizeof(query), "update %s set isTaken=1 where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, NULL);
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

int cancelSeat(int seatNumber, char *flightName)
{
    int rc, isTaken = 1;
    char cancelQuery[100];
    char *errorMessage;

    snprintf(cancelQuery, sizeof(cancelQuery), "select isTaken from %s where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, cancelQuery, seatTaken, (void *)&isTaken, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
    }
    if (!isTaken)
    {
        printf("Seat %d in flight %s is not taken, so it can't be canceled\n\n", seatNumber, flightName);
        return -1;
    }

    char query[100];
    snprintf(query, sizeof(query), "update %s set isTaken=0 where flightName=\"%s\" and seatNumber=\"%d\"", TABLENAME, flightName, seatNumber);
    rc = sqlite3_exec(airportDB, query, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL query error: %s\n\n", errorMessage);
        sqlite3_free(errorMessage);
        return -1;
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
        if(string[i] < '0' || string[i] > '9') return -1;
        result = result * 10 + (string[i] - '0');
    }

    return result;
}

void closeDatabase()
{
    sqlite3_close(airportDB);
    printf("Closed database\n");
}

int main(int argc, char **argv)
{
    char *command, *nameFlight;
    int seat;

    if(argc < 2) {
        printf("Wrong amount of arguments\n");
        return -1;
    }

    command = argv[1];
    nameFlight = argv[2];
    
    if(argc > 2) {
        seat = stringToInt(argv[3]);

        if(seat == -1 && strcmp(command, "cancel flight") != 0) {
            printf("Wrong number format on seat\n");
            return -1;
        }
    }

    switch (command)
    {
        case "create flight": createFlight(seat, nameFlight); break;
        case "cancel flight": cancelFlight(nameFlight); break;
        case "book": reserveSeat(seat, nameFlight); break;
        case "cancel seat": cancelSeat(seat, flightName); break;
        default: printf("Wrong command\n"); return -1;
    }
    return 0;
}