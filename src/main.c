#include "node.h"
#include "inondation.h"
#include <time.h>
#include <stdlib.h>
/***
	
	Ajouts :
		- Gestion d'un tlv permettant de changer la valeur de notre noeud encrypté avec RSA (utiliser change_msg pour cette fonctionnalité)
		- On ignore les réponses demandant un TLV request s'il n'a pas été envoyé (plutot genant dans certains cas mais bon c'est fait...)
		- Warning pour les hashs non corrects (Vérification de la cohérence des Node State) et d'autres warning pour les problèmes de paquets et IP local
		- Optimisation des calculs des hashs : les hashs ne sont calculé que lors de la modification d'une donnée ou lors de la création de celui ci (Calcul des hashes)
		- Lors de la reception d'un node hash : envoie direct de node state si seqno plus grand que celui recu (Calcul des hashes)
		- Lors de la reception d'un node hash : On envoie directement un node state request sans comparer les hashs si ont a un seqno plus faible (Calcul des hashes)
		- Un paquet ne dépasse pas 1024 octets, la limite que tout le monde devrait accépter en pratique (Agrégation)
		- A chaque paquet reçu on met toutes les réponses dans un meme paquet tant qu'on ne dépasse pas 1024 octets (Agrégation)
		- Lors de la reception d'un message, envoie la reponse à la même addresse (Adresses multiples)
		- On verifie que l'on envoie pas à une de nos adresses (sur le meme port) (Adresses multiples)
		- Ignore les adresses local au lien (Adresses locale au lien)
		- Si une requete est sans reponse, on redemande une reponse après 20 secondes (si le voisin ne disparait pas)
		- Chaque warning recu est stocké dans un fichier log_file sous la forme IP warning, ou IP correspond à l'IP qui a envoyé le warning
		- Traversé de NAT avec un serveur STUN pour recuperer son IPv4 (Traversée de Firewalls, de NAT)
	Extensions indiqués dans le sujet implémentés :
		- Vérification de la cohérence des Node State
		- Calcul des hashes
		- Agrégation
		- Adresses multiples
		- Adresses locales au lien
		- Traversée de Firewalls, de NAT

***/

int main(int argc, char** argv) {
	if(argc > 1 && strcmp(argv[1],"--help") == 0) {
		printf("Usage: dazibao_node {-s NUM} {-i ID} {-p PORT} {-f} {-n} {-d} {address port}*\n");
		printf("Launch a node\n");
		printf("OPTIONS : \n");
		printf("\t-s NUM : Set the node's first seqno as NUM (DEFAULT:%d)\n",DEFAULT_SEQNO);
		printf("\t-i ID : Set the id's node to ID (DEFAULT:%d)\n",DEFAULT_ID);
		printf("\t-p PORT : Indicate which port (different from %d) should be used (DEFAULT:%d)\n",CHANGE_PORT,DEFAULT_PORT);
		printf("\t-f : Run the process in background (fork)\n");
		printf("\t-n : The program will not add any neighbours at start\n");
		printf("\t-d : The node will broadcast the number of data in the network :\n"); 
		printf("\t\tThe data will look like : I see ____ data in this database \n");
		printf("Address and port given as arguments will be added as neighbours at the start (can be multiple)\n");
		printf("You should not give LOCAL LINK addresses\n");
		printf("If the address and the port are not specified and the -n option is not present, it'll initialize the node with %s:%d.\n",INIT_ADDR,INIT_PORT);
		exit(1);
	}
	
	srand((unsigned int) time(NULL));
	generate_keys();

	short opt_f = 0;
	short opt_n = 0;
	short opt_d = 0;

	
	uint16_t start_seqno = DEFAULT_SEQNO;
	uint64_t node_id = DEFAULT_ID;
	uint16_t myport = DEFAULT_PORT;
	int permanents = 0;
	for(int i = 1;i<argc;i++) {
		if(i != argc-1) {
			if(strcmp(argv[i],"-s") == 0) {
				start_seqno = atoi(argv[i+1]);
				i++;
			}
			else if(strcmp(argv[i],"-i") == 0) {
				
				node_id = atoll(argv[i+1]);
				printf("%" PRIu64 "\n",node_id);
				
				i++;
			}
			else if(strcmp(argv[i],"-p") == 0) {
				myport = atoll(argv[i+1]);
				i++;
			}
			else if(strcmp(argv[i],"-d") != 0 && strcmp(argv[i],"-n") != 0 && strcmp(argv[i],"-f") != 0) {
				permanents += addPermanent(argv[i],atoi(argv[i+1]));
				i++;
			}
		}
		if(strcmp(argv[i],"-d") == 0) {
			opt_d = 1;
		}
		else if(strcmp(argv[i],"-n") == 0) {
			opt_n = 1;
		}
		else if(strcmp(argv[i],"-f") == 0) {
			opt_f= 1;
		}
		
	}

	if(myport == CHANGE_PORT) {
		printf("port already taken for change messages\n");
		exit(1);
	}
	
	if(!opt_n) {
		if(permanents == 0) {
			permanents += addPermanent(INIT_ADDR,INIT_PORT);	
		}
		if(permanents == 0) exit(1);
		printf("Number of given interfaces : %d\n",permanents);
	}

	Node n = initNode(myport, start_seqno, node_id,opt_d,opt_n);

	printf("Seqno : %" PRIu16 "\nNode ID %" PRIu64" (" PrID ")\nPort : %" PRIu16 "\n", start_seqno,n->id,PrID_value(n->id),n->port);
	
	if(!opt_f || fork() == 0) {	
		startCommunication(n,myport);
	}
	freePermanents();
	
}