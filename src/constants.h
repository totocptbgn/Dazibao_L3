#ifndef CONSTANTS_H
#define CONSTANTS_H

/*

	STUN constants

*/
#define STUN_COOKIE 0x2112A442
#define XOR_MAPPED_ADDRESS_TYPE 0x0020

#define IPv4_ADDRESS_FAMILY 0x01
#define IPv6_ADDRESS_FAMILY 0x02
#define SERVERS {\
        {"stun.l.google.com" , 19302}, {"stun.l.google.com" , 19305},\
        {"stun1.l.google.com", 19302}, {"stun1.l.google.com", 19305},\
        {"stun2.l.google.com", 19302}, {"stun2.l.google.com", 19305},\
        {"stun3.l.google.com", 19302}, {"stun3.l.google.com", 19305},\
        {"stun4.l.google.com", 19302}, {"stun4.l.google.com", 19305},\
        {"stun.wtfismyip.com", 3478 }, {"stun.bcs2005.net"  , 3478 },\
        {"numb.viagenie.ca"  , 3478 }, {"173.194.202.127"   , 19302}}

#define NB_SERVERS 14
/*

	Default values

*/
#define DEFAULT_PORT 42069
#define DEFAULT_ID 666
#define DEFAULT_SEQNO 0
#define INIT_ADDR "jch.irif.fr"
#define INIT_PORT 1212
/*

	Print ID

*/
#define PrID "%08x%08x"
#define PrID_value(ID) ntohl(ID),ntohl(ID >> 32)
/*
	
	Taille des differents champs d'un tlv

*/
#define IP_SIZE 16
#define HASH_SIZE 16
#define ID_SIZE 8
#define PORT_SIZE 2
#define SEQNO_SIZE 2
#define TYPE_SIZE 1
#define LENGTH_SIZE 1
#define HEAD_SIZE (LENGTH_SIZE + TYPE_SIZE) //taille minimal pour les tlvs ( sauf pour pad1 )

/*
	Taille des tlvs
*/

#define PAD1_SIZE TYPE_SIZE
#define PADN_MIN_SIZE HEAD_SIZE
#define NEIGHBOUR_REQUEST_SIZE HEAD_SIZE
#define NEIGHBOUR_SIZE (HEAD_SIZE + IP_SIZE + PORT_SIZE)
#define NETWORK_HASH_SIZE (HEAD_SIZE + HASH_SIZE)
#define NETWORK_STATE_REQUEST_SIZE HEAD_SIZE
#define NODE_HASH_SIZE (HEAD_SIZE + ID_SIZE + SEQNO_SIZE + HASH_SIZE)
#define NODE_STATE_REQUEST_SIZE (HEAD_SIZE + ID_SIZE)
#define NODE_STATE_MIN_SIZE (HEAD_SIZE + ID_SIZE + SEQNO_SIZE + HASH_SIZE)
#define WARNING_MIN_SIZE HEAD_SIZE
#define STOCK_SIZE HEAD_SIZE

/*

	types des tlv

*/

#define PAD1_NUM 0
#define PADN_NUM 1
#define NEIGHBOUR_REQUEST_NUM 2
#define NEIGHBOUR_NUM 3
#define NETWORK_HASH_NUM 4
#define NETWORK_STATE_REQUEST_NUM 5
#define NODE_HASH_NUM 6
#define NODE_STATE_REQUEST_NUM 7
#define NODE_STATE_NUM 8
#define WARNING_NUM 9
#define CHANGE_MESSAGE_NUM 42
#define STOCK_NUM 51

/*
	taille composantes package
*/
#define MAGIC_SIZE 1
#define VERSION_SIZE 1
#define BODY_LENGTH_SIZE 2
#define PACKAGE_HEAD_SIZE (MAGIC_SIZE + VERSION_SIZE + BODY_LENGTH_SIZE)
/*
	Des limites
*/
#define MAX_ERROR_SIZE 192
#define MAX_DATA_SIZE 192
#define MAX_PACKAGE_SIZE 1024
/*
	Taille Hash
*/
#define HASH_SIZE 16
/*
	Les différents fichiers
*/
#define LOG_FILE "log_file"
#define DATA_FILE ".data"
/*
	port de modification des messages
*/
#define CHANGE_PORT 54321

/*
	Constantes pour les voisins
*/
#define MAX_VOISINS 15

#define MIN_VOISINS 5

/*

	print_data constants

*/

#define TIME_TO_WAIT 5

#endif
