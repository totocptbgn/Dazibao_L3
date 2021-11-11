#ifndef NODE_H
#define NODE_H

#include <time.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>

	   
typedef struct node* Node;
typedef struct voisin* Voisin;
typedef struct node* Node;

#include "database.h"
#include "history.h"
#include "tlv.h"
#include "stun_gestion.h"

/*

	Correspond à un voisin

*/
struct voisin {
	short permanent;
	time_t last_message;
	uint16_t port;
	unsigned char IP[IP_SIZE]; //pourquoi ne pas stocker dans un sock_addr?
	struct in6_addr SEND_IP;
};

/*

	Notre noeud avec les informations

*/
struct node {
	int socket;
	int bind_value;
	uint64_t id;
	uint16_t seq;
	uint16_t port;
	Voisin voisins[MAX_VOISINS];
	int nbvoisins;
	DataBase table;
	History history;
	char IPv4addr[16]; //mapped ipv4 of ourself
	short default_value; //opt d
	short neighbours; //opt n
};

/*
	Free all permanents
*/
void freePermanents(); 

short addPermanent(char* interface,uint16_t port);

Node initNode(uint16_t port, uint16_t seqno, uint64_t id_node,short opt_d,short opt_n);

/*
	Verifie si [IP] et [port] correspondent à un voisin
	si update != 0 alors on met a jour la date du dernier message reçu du voisin
*/
Voisin isVoisin(Node n,uint16_t port, unsigned char* IP,short update);

Voisin addVoisin(Node n, uint16_t port, unsigned char* IP);

#ifdef TEST_VOISINS
void printVoisins(Node n);
#endif

/*
	Enlève les voisins inactifs et renvoie le nombre de voisins supprimés
*/
int removeVoisinsInactifs(Node n);

/*
	Compare deux voisins renvoie 0 s'ils sont différents
*/
short cmpVoisins(Voisin a, Voisin b);

/*
	Verifie que [port] et [IP] ne correspont pas au noeud [n]
	renvoie 0 s'ils sont differents
*/
short mySelf(Node n, uint16_t port, unsigned char* IP);

/*
	Met a jour notre ip de conversation avec le voisin [v] à partir de [msg]
*/
short updateReceiveWith(struct msghdr msg,Voisin v);
#endif