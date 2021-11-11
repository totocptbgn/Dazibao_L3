/*

Creation des différents TLV avec son contenu
Convention : 
- NULL ou 0 en cas d'erreur
- modification du message de warning() en cas d'erreur

*/


#ifndef TLV_H
#define TLV_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include <arpa/inet.h>

//for tests
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include "database.h"


typedef void* TLV;

TLV pad1();

TLV padn(uint8_t n);

TLV neighbourRequest();

TLV neighbour(unsigned char* IP, uint16_t port);

TLV networkHash(unsigned char* hash);

TLV networkStateRequest();

TLV nodeHash(uint64_t ID, uint16_t seqno,unsigned char* nodehash);

TLV nodeStateRequest(uint64_t ID);

TLV nodeState(uint64_t ID, uint16_t seqno,unsigned char* hash,unsigned char* data,uint8_t sizedata);

TLV warning(unsigned char* message,int size);

//voir verify
TLV last_warning();

TLV change_message(unsigned char* message,int size);

TLV stock();

/*

	Fonctions de récupération des informations

*/

uint8_t getType(TLV tlv);

unsigned char* getIP(TLV tlv);

uint16_t getPort(TLV tlv);

DataHash getHash(TLV tlv);

uint64_t getID(TLV tlv);

uint16_t getSeqno(TLV tlv);

unsigned char* getData(TLV tlv);

uint8_t getSizeData(TLV tlv);

//le message a pour taille la taille du contenu
unsigned char* getMessage(TLV tlv);

//taille du contenu : avant d'utiliser cette fonction verifier que 2 octets sont disponibles ou que le type est 0
uint8_t getLengthTLV(TLV tlv);

//taille total
uint8_t size(TLV tlv);

/*
	verifie le contenu du tlv (mais pas sa taille)
	renvoie 1 s'il est correct
	sinon renvoie 0 et met à jour last_warning
*/
short verify(TLV tlv);

//verifie si une IP est non local et renvoie 1 si c'est le cas
short verifyIP(unsigned char IP[16]);

#endif