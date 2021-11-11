
/*
	Gestion historique :
		- recherche ( searchInHistory )
		- suppression ( removeInHistory )
		- ajout ( addInHistory )
	Les arguments des fonctions :
		- type -> type du tlv ( -1 si pas de specification )
		- act -> action realisé par le node sur le message : envoyé ou recu ( -1 si pas de spécification )
		- id -> id contenue dans le tlv ( -1 si pas de specification )
		- v -> avec qui ( NULL si pas de specification )		
		
*/

#ifndef HISTORY_H
#define HISTORY_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tlv.h"

typedef struct history* History;

#include "node.h"
/*

	Permet de savoir si on a reçue ou envoyée un message dans l'historique

*/
enum action {
	SENT, RECEIVE, ALL = -1 /* -1 permet de ne pas différencier les infos dans l'historique */
};

typedef enum action action;



#ifdef TEST_HISTORY
void printHistory(History h);
#endif


/*
	Recherche dans l'historique :
	on peut specifié le type de tlv, l'action effectué, l'id contenu dans le tlv, le voisin avec lequel on a communiqué
	Renvoie le TLV trouvé, NULL sinon
	si [type] = -1 alors il n'est pas spécifié, 
	si [action] = -1 alors il n'est pas specifié, 
	si id = -1 alors il n'est specifié, 
	si v = NULL alors il n'est spécifié

*/
void* searchInHistory(History h, int8_t type, action act, int64_t id, Voisin v);

/*
	Suppression dans l'historique : les specifications des messages supprimés sont ceux indiqué dans l'entete
	En cas d'echec 0 sinon le nombre de messages supprimés
	avec multiple = 0 : maximum 1 message supprimé
*/
int removeInHistory(History h, int8_t type, action act,int64_t id, Voisin v, short multiple);

/*

	Ajoute un TLV [tlv] à l'historique, 
	[v] correspond au voisin avec lequel on communique,
	act l'action du TLV
	-1 en cas d'echec

*/
short addInHistory(History h, action act, Voisin v, TLV tlv);

History initHistory();

void freeHistory(History h);

/*
	
	Iterateur

*/
typedef struct historyIterator* HistoryIterator;

/*

	Initialise un itérateur d'historique avec des conditions sur les messages si voulu
	(même conditions que pour searchInHistory)

*/
HistoryIterator initHistoryIterator(History h, int8_t type, action act,int64_t id, Voisin v);

/*
	Indique si prochain TLV
*/
short hasNextTLVMessage(HistoryIterator hi);

/*
	Renvoie prochain TLV
*/
TLV getNextTLVMessage(HistoryIterator hi);
#endif