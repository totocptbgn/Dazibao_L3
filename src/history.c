#include "history.h"

/*
	TLV dans l'historique
*/

struct message {
	action act;
	TLV tlv;
	Voisin pair;
	time_t time;
};

typedef struct message* Message;

struct history {
	Message* content;
	int nbmessage;
};




#ifdef TEST_HISTORY
void printHistory(History h) {
	printf("--- HISTORY ---\n");
	printf("History Size : %d\n", h -> nbmessage);
	printf("TLV : \n");
	for(int i = 0;i < h -> nbmessage; i++) {
		printf("- TLV : %" PRIu8 " of size %" PRIu8 "\n",getType(h -> content[i] -> tlv),size(h -> content[i] -> tlv));
	}
	printf("--- ------- ---\n");
}
#endif

void* searchInHistory(History h, int8_t type, action act, int64_t id, Voisin v) {
	for(int i = 0; i < h -> nbmessage; i++) {
		if( (type == -1 || getType(h -> content[i] -> tlv) == type) && ( act == -1 || h -> content[i] -> act == act) && (v==NULL || cmpVoisins(h -> content[i] -> pair, v) ) && (id == -1 || getID(h -> content[i] -> tlv) == id) ) {
			return h -> content[i] -> tlv;
		}			
	}
	
	return NULL;
}

//libère un message
static void freeMessage(Message m) {
	free(m -> tlv);
	free(m);
}

int removeInHistory(History h, int8_t type, action act,int64_t id, Voisin v, short multiple) {
	int number = 0;
	for(int i = 0; i < h -> nbmessage; i++) {
		if((type == -1 || getType(h -> content[i]-> tlv) == type) && (act == -1 || h -> content[i]-> act == act) && (v == NULL || cmpVoisins(h -> content[i] -> pair, v)) && (id == -1 || getID(h -> content[i] -> tlv) == id)) {
			freeMessage(h -> content[i]);
			
			memmove(h -> content + i,h -> content + i + 1,sizeof(Message) * ( h -> nbmessage - i - 1) );
			
			h -> nbmessage--;
			if(!multiple) {
				#ifdef TEST_HISTORY
					printf("HISTORY : remove %d tlv\n",1);
					printf("HISTORY : size %d\n",h -> nbmessage);
				#endif
				return 1;
				
			}
			else {
				i--;
				number++;
			}
		}			
	}
	#ifdef TEST_HISTORY
		printf("HISTORY : remove %d tlv\n",number);
		printf("HISTORY : size %d\n",h -> nbmessage);
	#endif
	return number;
}

short addInHistory(History h, action act, Voisin v, TLV tlv) {
	if(h -> content == NULL) {
		h -> content = malloc(sizeof(Message));
		if( h -> content == NULL) {
			#ifdef TEST_MEMORY
				printf("MEMORY : malloc history content failed\n");
			#endif
			return 0;
		}
	}
	else {
		void* newpointeur;
		if((newpointeur = realloc(h -> content,sizeof(Message) * (h -> nbmessage + 1))) == NULL) {
			#ifdef TEST_MEMORY
				printf("MEMORY : realloc history content failed\n");
			#endif
			return 0;
		}
		h -> content = newpointeur;
	}
	Message newmessage = malloc(sizeof(struct message));
	if(newmessage == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc history message failed\n");
		#endif
		return 0;
	}
	newmessage -> pair = v;
	newmessage -> tlv = tlv;
	newmessage -> act = act;
	newmessage -> time = time(NULL);
	h -> content [h -> nbmessage] = newmessage;
	h -> nbmessage ++;
	#ifdef TEST_HISTORY
		printf("HISTORY : add 1 tlv\n");
		printf("HISTORY : size %d\n",h -> nbmessage);
	#endif
	
	return 1;
}

History initHistory() {
	History h = malloc(sizeof(struct history));
	if(h == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc history failed\n");
		#endif
		return NULL;
	}
	h -> content = NULL;
	h -> nbmessage = 0;
	return h; 
}

void freeHistory(History h) {
	for(int i = 0; i < h -> nbmessage; i++) {
		free(h -> content[i] -> tlv); 
		free(h -> content[i]);
	}
	free(h -> content); 
	free(h);
}

/*
	Iterateur de data
*/
struct historyIterator {
	History h;
	int current;
	int8_t type;
	action act;
	int64_t id;
	Voisin v;
};

HistoryIterator initHistoryIterator(History h, int8_t type, action act,int64_t id, Voisin v) {
	HistoryIterator hi = malloc(sizeof(struct historyIterator));
	if(hi == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc history iterator failed\n");
		#endif
		return NULL;
	}
	hi -> h = h;
	hi -> current = 0;
	hi -> type = type;
	hi -> act = act;
	hi -> id = id;
	hi -> v = v;
	return hi;
}


TLV getNextTLVMessage(HistoryIterator hi) {
	while( hi -> current < hi -> h -> nbmessage ) {
		
		if( (hi -> type == -1 || getType(hi -> h -> content[hi -> current] -> tlv) == hi -> type) 
			&& (hi -> act == -1 || hi -> h -> content[hi -> current] -> act == hi -> act) 
			&& (hi -> v == NULL || hi -> h -> content[hi -> current] -> pair == hi -> v ) 
			&& (hi -> id == -1 || getID(hi -> h -> content[hi -> current] -> tlv) == hi -> id) ) {
			Message m = hi -> h -> content[hi -> current] -> tlv;
			hi -> current++;
			return m;
		}			
		hi -> current++;
	}
	return NULL;
}

short hasNextTLVMessage(HistoryIterator hi) {
	for(int i = hi -> current; i < hi -> h -> nbmessage; i++) {
		if( (hi -> type == -1 || getType(hi -> h -> content[i] -> tlv) == hi -> type) 
			&& (hi -> act == -1 || hi -> h -> content[i] -> act == hi -> act) 
			&& (hi -> v==NULL || hi -> h -> content[i] -> pair == hi -> v ) 
			&& (hi -> id == -1 || getID(hi -> h -> content[i] -> tlv) == hi -> id) ) {
			return 1;
		}			
	}
	return 0;
}


