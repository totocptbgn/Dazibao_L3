#ifndef DATABASE_H
#define DATABASE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef TEST_DATA
	#include <unistd.h>
#endif
#include "constants.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <unistd.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/*

	Correspond à une donnée de la base de donnée
	
*/

typedef unsigned char* DataHash;

struct data {
	uint64_t id;
	uint16_t seqno;
	void* content;
	int size;
	unsigned char hash_data[HASH_SIZE];
};

typedef struct database* DataBase;

typedef struct data* Data;

/*
	Creer une donnée venant du noeud d'id [ID]
	Ne met pas à jour le hash_data
*/
Data initData(uint64_t id);

/*
	Met à jour la donnée, si [seqno] = -1, ne met pas à jour la data
	si [size] = -1, ne met pas a jour le contenu de la data
	Le node hash est mis à jour si nécessaire
*/
short updateData(Data d,uint16_t seqno,void* content,int size_content);

/*
	Recupère le network hash de la database (sans le recalculer)
*/
DataHash getNetworkHash(DataBase dh);

/*
	Libère l'espace mémoire d'une Data
*/
void freeData(Data d);

/*
	Initialise une database avec la donnée d
*/
DataBase initDataBase(Data d);

/*
	Stocke dans un fichier les données de la database sous la forme
	ID message
	Un par ligne

*/
void stockDatabase(DataBase db);

/*
	Permet de changer notre propre donnée dans la database [db] : 
	renvoie 1 si elle est mis à jour, 0 sinon
	La data [d] doit avoir l'id de notre noeud
	La data donnée n'a pas besoin d'etre libéré dans tout les cas
*/
short updateMyData(DataBase db,Data d);

/*
	Renvoie le nombre de donnée dans la database [db]
*/
int getNbData(DataBase db);

/*
	met a jour la donnée dans [base] si celle-ci doit l'etre : renvoie 1 si modifié 0 sinon
*/

short updateDataBase(DataBase base, Data d,uint64_t id);

/*
	Renvoie la data dans la database [db] en rapport avec le numero d'i-noeud [id]
*/
Data getDataBase(DataBase db,uint64_t id);

// iterateur des données
typedef struct dataIterator* DataIterator;

//création de l'iterateur sur la database [d]
DataIterator initDataIterator(DataBase d);

//verifie qu'une prochaine donnée existe
short hasNextData(DataIterator di);

//donne la prochaine donnée
Data getNextData(DataIterator di);

#ifdef TEST_DATA
	//affiche l'ensemble de la database [db]
	void printDataBase(DataBase db);
#endif
#endif