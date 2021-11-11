#ifndef STU_GESTION_H
#define STU_GESTION_H

#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

struct STUNServer
{
    char* address;
    
    uint16_t port;
};

/*
    Get the external IPv4 address
    By giving a STUNServer [server], 
    It will place our IPv4 mapped address in [ address ]
    return 0 if success, 1 if not found and < 0 if a bug occured
*/
int getPublicIPv4Address(struct STUNServer server, char address[IP_SIZE]);

#endif