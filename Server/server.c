#include <server.h>

sqlite3 *airportDB;

//gcc server.c -pthread -l sqlite3

//Only one table called flights contains all the airport data. Command used to create it is given below:
//create table flights (flightName string, seatNumber int, isTaken int, primary key (flightName,seatNumber));

int main(int argc, char **argv)
{
    char *errorMessage;

    int fd, fd2, longitud_cliente, puerto;

    //Abro/creo airport.db
    int rc = sqlite3_open("airport.db", &airportDB);
    if (rc)
    {
        fprintf(stderr, "\nCan't open database: %s\n\n", sqlite3_errmsg(airportDB));
        return 0;
    }
    else
    {
        fprintf(stderr, "\nOpened database successfully\n");
    }

    //Crea tabla flights si no existe
    char createFlights[256] = "create table if not exists flights (flightName string, seatNumber int, isTaken int, primary key (flightName,seatNumber))";
    rc = sqlite3_exec(airportDB, createFlights, NULL, NULL, NULL);
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

    puerto = PUERTO;

    struct sockaddr_in server;
    struct sockaddr_in client;

    //Configuracion del servidor
    server.sin_family = AF_INET;         //Familia TCP/IP
    server.sin_port = htons(puerto);     //Puerto
    server.sin_addr.s_addr = INADDR_ANY; //Cualquier cliente puede conectarse
    bzero(&(server.sin_zero), 8);        //Funcion que rellena con 0's

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error definiendo el socket");
        exit(-1);
    }

    if (bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("error en bind() \n");
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

        // Aca el servidor envia el mensaje que queramos.
        //El 2do parametro es el mensaje y el 3ro la longitud.
        char *command, *nameFlight, *seat;
        if (fork() == 0)
        {
            execl("./requestHandler", "./requestHandler", command, nameFlight, seat, (char *)NULL);
        }
        send(fd2, "Bienvenido a mi servidor.\n", 26, 0);
        close(fd2); /* cierra fd2 */
    }

    close(fd);

    //aca atiende a los clientes con forks y threads. Muchas funciones de sql creadas aca
    //van a tener que moverse a un requestHandler.c a donde mandemos a cada cliente

    //si los querys estan bien no tira error, pero no avisa que esta mal

    closeDatabase();

    return 0;
}