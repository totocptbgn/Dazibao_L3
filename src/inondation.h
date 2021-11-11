#ifndef INONDATION_H
#define INONDATION_H

#include <arpa/inet.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include "constants.h"
#include "node.h"
#include "history.h"
#include "tlv.h"
#include "package.h"
#include "database.h"
#include "crypt.h"




void startCommunication(Node n,uint16_t port);

#endif