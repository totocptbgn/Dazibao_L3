/*
	Gestion et Creation d'un paquet
	Convention : 
	- NULL ou < 0 en cas d'erreur
	Erreurs :
	...
*/

#ifndef PACKAGE_H
#define PACKAGE_H
#include <stdlib.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include "tlv.h"

typedef struct package* Package;

unsigned char* getPackageConstructionError();

/* Retourne le numéro de Version d'un paquet */
uint8_t getVersion(Package paquet);

/* Retourne le numéro de Magic d'un paquet */
uint8_t getMagic(Package paquet);

/* Retourne la taille en octets  d'un paquet */
uint16_t getPackageLength(Package paquet);

/* Permet de savoir si un tlv de type type, contenant l'id id est dans le package, -1 si pas de specification */
short inside(Package paquet, int8_t type,int64_t id);
short isEmpty(Package p);
/*
	Envois
*/

/* Création d'un paquet à partir d'un message recu */
Package new_paquet();

/** 
 * Ajout d'un TLV au paquet
 * Valeurs de retour :
 * 		0 : Succès
 *     -1 : Plus de place
 * 	   -2 : Erreur de realloc()
 */
int addTLV(Package paquet, TLV tlv);

/* Donne le message à envoyé et free le paquet et les TLV non ajouté à l'historique (sans Request dans le nom) */
void* build(Package paquet);

/*
	Receptions	
*/

/* Construction d'un package à partir d'un paquet recu, put warnings in warningPackage */
Package paquet(void* buf, uint16_t length,Package warningPackage);

/* detruit entièrement un paquet */
void destroyPackage(Package package);
/** Obtenir le TLV suivant à la façon d'un itérateur
 *  Valeurs de retour :
 * 		NULL : Plus de TLVs
 * 		Sinon, renvoie un pointeur vers le TLV.
 */
TLV getNextTLV(Package paquet);


/* Recommence la lecture des TLVs depuis le début */
void rewind_package(Package paquet); 
#endif
