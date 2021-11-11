#include "package.h"

struct package {
	uint8_t magic; 			// Magic
	uint8_t version;		// Version
	uint16_t body_length; 	// Taille en octets
	TLV* body;		// Corps du paquet
	int cur;			// Pointeur courant
	int size; 			// Nombre de TLV
} typedef package;

static unsigned char package_construction_error[1024] = "";

unsigned char* getPackageConstructionError() {
	if(strlen((char*)package_construction_error) == 0) {
		return NULL;
	}
	else return package_construction_error;
}

uint8_t getVersion(Package paquet) {
	return paquet -> version;
}

uint8_t getMagic(Package paquet) {
	return paquet -> magic;
}

uint16_t getPackageLength(Package paquet) {
	return paquet -> body_length + PACKAGE_HEAD_SIZE;
}

Package new_paquet() {
	Package pqt = malloc(sizeof(package));
	if(pqt == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc empty package failed\n");
		#endif
		return NULL;
	}
	pqt->magic = 95;
	pqt->version = 1;
	pqt->body_length = 0;
	pqt->body = NULL;
	pqt->cur = 0;
	pqt->size = 0;
	return pqt;
}

int addTLV(Package paquet, TLV tlv) {
	struct package* p = paquet;
	if (p ->body_length + size(tlv) > MAX_PACKAGE_SIZE - PACKAGE_HEAD_SIZE) {
		#ifdef TEST_DEBUG
			printf("DEBUG : reached package size maximal with %" PRIu8"\n",p->body_length + size(tlv));
		#endif
		return -1;
	}
	uint8_t type = getType(tlv);
	int pos = 0;
	while(pos < p -> size && getType(p -> body [pos]) > type )
		pos++;

	while(pos != p -> size && getType(p -> body [pos]) == type) {
		if(type == NODE_HASH_NUM || type == NODE_STATE_REQUEST_NUM || type == NODE_STATE_NUM ) {
			if(getID(tlv) == getID(p -> body[pos])) {
				#ifdef TEST_DEBUG
					printf("DEBUG : TLV already inside\n");
				#endif
				return 1;
			}
		}
		else {
			#ifdef TEST_DEBUG
				printf("DEBUG : TLV already inside\n");
			#endif
			return 1;
		}
		pos ++;
	}
	void* rea = realloc(p -> body, (p -> size + 1) * sizeof(TLV));
	if (rea == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : realloc body package failed\n");
		#endif
		return -2;
	}
	
	p -> body = rea;
	p -> body_length += size(tlv);
	
	/*
		Ordre decroissant tlv pour eviter de supprimer de l'historique 
		( et donc le free ) une donnée qui vient d'être ajouté et doit être envoyé
	*/
	

	memmove( p -> body + (pos + 1), p -> body + pos, (p -> size - pos) * sizeof(TLV));
	p -> body[pos] = tlv;
	p -> size++;
	p -> body = rea;
	return 0;
}

TLV getNextTLV(Package paquet) {
	
	struct package* p = paquet;
	if (p->cur >= p->size) {
		return NULL;
	}
	p->cur++;
	return p->body[p->cur - 1];
}

void rewind_package(Package paquet) {
	((struct package*) paquet) -> cur = 0; 
}

void destroyPackage(Package package) {
	for(int i = 0; i < package -> size; i++ ) {
		free(package -> body[i]);
	}
	free(package -> body);
	free(package);
}

Package paquet(void* buf, uint16_t length,Package warningPackage) {
	package_construction_error[0] = '\0';
	if(length < 4) {
		return NULL;
	}
	
	uint8_t magic = *((uint8_t*)buf);
	
	#ifdef TEST_RECEIVE
		printf("--- RECEIVE ---\n");
		printf("-> Magic : %" PRIu8 "\n",magic);
	#endif
	if(magic != 95) {
		#ifdef TEST_RECEIVE
			printf("Debug : error package package magic\n");
		#endif
		return NULL;
	}
	
	uint8_t version = *((uint8_t*)buf + MAGIC_SIZE);
	#ifdef TEST_RECEIVE
		printf("-> Version : %" PRIu8 "\n",version);
	#endif
	if(version != 1) {
		#ifdef TEST_DEBUG
			printf("DEBUG : unexcepted version\n");
		#endif
		return NULL;
	}
	
	uint16_t contentsize = ntohs(*((uint16_t*)(buf+(MAGIC_SIZE+VERSION_SIZE))));

	#ifdef TEST_RECEIVE
		printf("-> Size : %" PRIu16 "\n",contentsize);
	#endif

	if(contentsize + PACKAGE_HEAD_SIZE > length) {
		#ifdef TEST_DEBUG
			printf("DEBUG : unexcepted size %" PRIu16 ", should be %" PRIu16 "\n",contentsize,length - 4);
		#endif
		snprintf((char*)package_construction_error,1024,"-: Excepted Package size <= %" PRIu16 " but found %" PRIu16,length - PACKAGE_HEAD_SIZE,contentsize);
		return NULL;
	}
	
	
	Package package = new_paquet();
	void* current = buf + PACKAGE_HEAD_SIZE;
	while(contentsize != 0) {
		uint8_t siz;
		if(contentsize == 1) {
			if(getType(current) != PAD1_NUM) {
				#ifdef TEST_DEBUG
					printf("DEBUG : size 1 tlv not TLV PAD1\n");
				#endif
				snprintf((char*)package_construction_error,1024,"-: Atempt to use a TLV of size 1 != PAD1");
				destroyPackage(package);
				return NULL;
			}
			siz=1;
		}
		else {
			siz = size(current);
			
			if(siz > contentsize) {
				#ifdef TEST_DEBUG
					printf("DEBUG : package size too big\n");
				#endif
				snprintf((char*)package_construction_error,1024,"-: Size of a TLV exceed package size");
				destroyPackage(package);
				return NULL;
			}

			TLV tlv = NULL;

			if(!verify(current)) {
				tlv = last_warning();
				if(tlv == NULL || warningPackage == NULL || addTLV(warningPackage,tlv) != 0) {
					#ifdef TEST_DEBUG
						printf("DEBUG : TLV Warning not added\n");
					#endif
				}
				else {
				#ifdef TEST_WARNING
					printf("WARNING : send Warning : ");
					fflush(stdout);
					write(1,getMessage(tlv),getLengthTLV(tlv));
					printf("\n");
				#endif
				}
				current+=siz;
				contentsize-=siz;
				#ifdef TEST_DEBUG
					printf("DEBUG : verify failed\n");
				#endif
				continue;
			}
			
			#ifdef TEST_RECEIVE
				printf("-> TLV : type %" PRIu8 " size %" PRIu8 "\n",getType(current),size(current));
				char IP[100];
			#endif

			switch(getType(current)) {
				case PAD1_NUM:
					tlv = pad1();
					break;
				case PADN_NUM:
					tlv = padn(getLengthTLV(current));
					break;
				case NEIGHBOUR_REQUEST_NUM:
					tlv = neighbourRequest();
					break;
				case NEIGHBOUR_NUM:
					#ifdef TEST_RECEIVE
						inet_ntop(AF_INET6, getIP(current),IP, 100);
						printf("\tIP receive : %s\n",IP);
						printf("\tPort receive : %" PRIu16 "\n",getPort(current));
					#endif
					tlv = neighbour(getIP(current),getPort(current));
					break;
				case NETWORK_HASH_NUM:
					#ifdef TEST_RECEIVE
						printf("\tNetwork Hash receive : ");
						for(int i = 0;i < HASH_SIZE;i++) {
							printf("%02x",getHash(current)[i]);
						}
						printf("\n");
					#endif
					tlv = networkHash(getHash(current));
					break;
				case NETWORK_STATE_REQUEST_NUM:
					tlv = networkStateRequest();
					break;
				case NODE_HASH_NUM:
					#ifdef TEST_RECEIVE
						printf("\tID receive : " PrID "\n",PrID_value(getID(current)));
						printf("\tSeqno receive : %" PRIu16 "\n",getSeqno(current));
						printf("\tNode Hash receive : ");
						for(int i = 0;i < HASH_SIZE;i++) {
							printf("%02x",getHash(current)[i]);
						}
						printf("\n");
					#endif
					tlv = nodeHash(getID(current),getSeqno(current),getHash(current));
					break;
				case NODE_STATE_REQUEST_NUM:
					#ifdef TEST_RECEIVE
						printf("\tID receive : " PrID "\n",PrID_value(getID(current)));
					#endif
					tlv = nodeStateRequest(getID(current));
					break;
				case NODE_STATE_NUM:
					#ifdef TEST_RECEIVE
						printf("\tID receive : " PrID "\n",PrID_value(getID(current)));
						printf("\tSeqno receive : %" PRIu16 "\n",getSeqno(current));
						printf("\tData receive : \n\t");
						fflush(stdout);
						write(1,getData(current),getSizeData(current));
						printf("\n");
					#endif
					tlv = nodeState(getID(current),getSeqno(current),getHash(current),getData(current),getSizeData(current));
					break;
				case WARNING_NUM:
					#ifdef TEST_RECEIVE
						printf("\tMessage receive : \n\t");
						fflush(stdout);
						write(1,getMessage(current),getLengthTLV(current));
						printf("\n");
					#endif
					tlv = warning(getMessage(current),getLengthTLV(current));
					break;
				case CHANGE_MESSAGE_NUM:
					#ifdef TEST_RECEIVE
						printf("\tMessage receive : \n\t");
						for(int i = 0; i < getLengthTLV(current); i++) {
							printf("%02x",getMessage(current)[i]);
						}
						printf("\n");
					#endif
					tlv = change_message(getMessage(current),getLengthTLV(current));
					break;
				case STOCK_NUM:
					tlv = stock();
				default:
					#ifdef TEST_DEBUG
						printf("DEBUG : TLV type unknown\n");
					#endif
					break;
			}
			if(tlv == NULL || addTLV(package,tlv) != 0) {
				#ifdef TEST_DEBUG
					printf("Debug : TLV not added\n");
				#endif
			}
			current+=siz;
			contentsize-=siz;
		}
		
	}
	#ifdef TEST_RECEIVE
		printf("-> Effective Size %" PRIu16 "\n",((struct package*) package)->body_length + PACKAGE_HEAD_SIZE);
		printf("--- ------- ---\n");
	#endif
	#ifdef TEST_PAUSE
		
		getchar();
	#endif
	return package; 
}

short isEmpty(Package paquet) {
	return ((struct package*) paquet) -> size == 0;
}

short inside(Package paquet, int8_t type,int64_t id) {
	struct package* p = paquet;
	for(int i = 0; i < p -> size; i++ ) {
		if( (type == -1 || getType(p -> body[i]) == type) && (id == -1 || getID(p -> body[i]) == id) ) {
			return 1;
		}
	}
	return 0;
}

void* build(Package paquet) {
	struct package* p = paquet;
	
	int total_size = PACKAGE_HEAD_SIZE;
	
	total_size += p -> body_length;

	void* res = malloc(total_size);
	if(res == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc build package failed\n");
		#endif
		return NULL;
	}
	memcpy(res,&(p -> magic),MAGIC_SIZE);
	
	memcpy(res+MAGIC_SIZE,&(p -> version),VERSION_SIZE);
	
	uint16_t siz = htons(p -> body_length);
	
	memcpy(res+MAGIC_SIZE+VERSION_SIZE,&siz,BODY_LENGTH_SIZE);
	#ifdef TEST_SEND
		printf("--- SEND ---\n");
		printf("-> Magic : %" PRIu8 "\n",p -> magic);
		printf("-> Version : %" PRIu8 "\n",p -> version);
		printf("-> Size : %" PRIu16 "\n",ntohs(siz));
	#endif
	uint16_t current = PACKAGE_HEAD_SIZE;
	for(int i = 0; i < p -> size;i++) {
		#ifdef TEST_SEND
			printf("-> TLV :  type %" PRIu8 " of size %" PRIu8 "\n",getType(p -> body[i]), size(p -> body[i]));
		#endif
		int size_tlv = size(p -> body[i]);
		memcpy(res+current,p -> body[i],size_tlv);

		current += size_tlv;
		//on free les packets non ajouté dans l'historique
		if(getType(p -> body[i]) == PAD1_NUM 
		|| getType(p -> body[i]) == PADN_NUM 
		|| getType(p -> body[i]) == WARNING_NUM
		|| getType(p -> body[i]) == NETWORK_HASH_NUM
		|| getType(p -> body[i]) == NEIGHBOUR_NUM
		|| getType(p -> body[i]) == NODE_STATE_NUM
		|| getType(p -> body[i]) == NODE_HASH_NUM
		|| getType(p -> body[i]) == CHANGE_MESSAGE_NUM
		|| getType(p -> body[i]) == STOCK_NUM
		) {
			free(p -> body[i]);
		}
	}
	free(p ->body);
	free(paquet);

	#ifdef TEST_SEND
		printf("--- ---- ---\n");
	#endif
	#ifdef TEST_PAUSE
		getchar();
	#endif
	
	return res;
	
}