#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define IP "127.0.0.1"
#define PUERTO 6667
#define PACKAGESIZE 1024

int sendMessage();

#endif