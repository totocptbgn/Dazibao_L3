#include "tlv.h"

static unsigned char* last_message[MAX_ERROR_SIZE];

TLV pad1() {

	void* content = calloc(PAD1_SIZE,1);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Pad1 failed\n");
		#endif
		return NULL;
	}
	return content;
}

TLV padn(uint8_t n) {
	void* content = calloc(n + PADN_MIN_SIZE,1);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Padn failed\n");
		#endif
		return NULL;
	}
	uint8_t type = PADN_NUM;
	memmove(content, &type,TYPE_SIZE);
	memmove(content + TYPE_SIZE, &n,LENGTH_SIZE);
	return content;
}

TLV neighbourRequest() {
	
	void* content = malloc(NEIGHBOUR_REQUEST_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Neighbour Request failed\n");
		#endif
		return NULL;
	}
	
	uint8_t type = NEIGHBOUR_REQUEST_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = NEIGHBOUR_REQUEST_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE,&size,LENGTH_SIZE);
	return content;
}

TLV neighbour(unsigned char* IP, uint16_t port) {
	port = htons(port);
	void* content = malloc(NEIGHBOUR_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Neighbour failed\n");
		#endif
		return NULL;
	}
	uint8_t type = NEIGHBOUR_NUM;
	memmove(content, &type,TYPE_SIZE);

	uint8_t size = NEIGHBOUR_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE,&size,LENGTH_SIZE);

	memmove(content+HEAD_SIZE,IP,IP_SIZE);
	memmove(content+HEAD_SIZE+IP_SIZE,&port,PORT_SIZE);
	return content;
}

TLV networkHash(unsigned char* hash) {
	void* content = malloc(NETWORK_HASH_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Network Hash failed\n");
		#endif
		return NULL;
	}
	uint8_t type = NETWORK_HASH_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = NETWORK_HASH_SIZE - HEAD_SIZE;
	memmove(content + TYPE_SIZE,&size,LENGTH_SIZE);
	memmove(content + HEAD_SIZE,hash,HASH_SIZE);
	return content;
}

TLV networkStateRequest() {
	void* content = malloc(NETWORK_STATE_REQUEST_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Network State Request failed\n");
		#endif
		return NULL;
	}
	uint8_t type = NETWORK_STATE_REQUEST_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = NETWORK_STATE_REQUEST_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE,&size,LENGTH_SIZE);
	return content;
}

TLV nodeHash(uint64_t ID, uint16_t seqno,unsigned char* nodehash) {
	seqno = htons(seqno);
	void* content = malloc(NODE_HASH_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Node Hash failed\n");
		#endif
		return NULL;
	}
	uint8_t type = NODE_HASH_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = NODE_HASH_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE,&ID,ID_SIZE);
	memmove(content+HEAD_SIZE+ID_SIZE,&seqno,SEQNO_SIZE);
	memmove(content+HEAD_SIZE+ID_SIZE+SEQNO_SIZE,nodehash,HASH_SIZE);
	return content;
}

TLV nodeStateRequest(uint64_t ID) {
	void* content = malloc(NODE_STATE_REQUEST_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Node State Request failed\n");
		#endif
	}
	uint8_t type = NODE_STATE_REQUEST_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = NODE_STATE_REQUEST_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE, &ID,ID_SIZE);
	return content;
}


TLV nodeState(uint64_t ID, uint16_t seqno, unsigned char* hash, unsigned char* data,uint8_t sizedata) {
	seqno = htons(seqno);
	if(sizedata > MAX_DATA_SIZE) {
		#ifdef TEST_MEMORY
			printf("MEMORY : size data for TLV Node State to high\n");
		#endif
		return NULL;
	}
	void* content = malloc(sizedata + NODE_STATE_MIN_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Node State failed\n");
		#endif
	}
	uint8_t type = NODE_STATE_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = sizedata + NODE_STATE_MIN_SIZE - HEAD_SIZE;
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE, &ID,ID_SIZE);
	memmove(content+HEAD_SIZE+ID_SIZE, &seqno,SEQNO_SIZE);
	memmove(content+HEAD_SIZE+ID_SIZE+SEQNO_SIZE, hash,HASH_SIZE);
	memmove(content+HEAD_SIZE+ID_SIZE+SEQNO_SIZE+HASH_SIZE,data,sizedata);
	return content;

}

TLV warning(unsigned char* message,int size_message) {
	if(size_message > MAX_ERROR_SIZE) {
		#ifdef TEST_MEMORY
			printf("MEMORY : size data for TLV Warning to high\n");
		#endif
		return NULL;
	}
	void* content = malloc(HEAD_SIZE+size_message);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Warning failed\n");
		#endif
		return NULL;
	}
	uint8_t type = WARNING_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = size_message;
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE, message,size);
	return content;
}

TLV last_warning() {
	void* content = malloc(HEAD_SIZE+strlen((char*)last_message));
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Warning failed\n");
		#endif
	}
	uint8_t type = WARNING_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = strlen((char*)last_message);
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE, last_message,size);
	return content;
}

TLV change_message(unsigned char* message, int size_message) {
	if(size_message > MAX_ERROR_SIZE) {
		#ifdef TEST_MEMORY
			printf("MEMORY : size data for TLV Change Message to high\n");
		#endif
		return NULL;
	}
	void* content = malloc(HEAD_SIZE+size_message);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Change Message failed\n");
		#endif
		return NULL;
	}
	uint8_t type = CHANGE_MESSAGE_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = size_message;
	memmove(content+TYPE_SIZE, &size,LENGTH_SIZE);
	memmove(content+HEAD_SIZE, message,size);
	return content;
}

TLV stock() {
	void* content = malloc(STOCK_SIZE);
	if(content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc TLV Network State Request failed\n");
		#endif
		return NULL;
	}
	uint8_t type = STOCK_NUM;
	memmove(content, &type,TYPE_SIZE);
	uint8_t size = STOCK_SIZE - HEAD_SIZE;
	
	memmove(content+TYPE_SIZE,&size,LENGTH_SIZE);
	return content;
}

uint8_t getType(TLV tlv) {
	uint8_t p = 0;
	memcpy(&p,tlv,1);
	return p;

}

unsigned char* getIP(TLV tlv) {
	if(getType(tlv) == NEIGHBOUR_NUM) {
		return tlv+HEAD_SIZE;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getIP on bad TLV\n");
	#endif
	return NULL;
}

unsigned char* getMessage(TLV tlv) {
	if(getType(tlv) == WARNING_NUM || getType(tlv) == CHANGE_MESSAGE_NUM) {
		return tlv+HEAD_SIZE;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getMessage on bad TLV\n");
	#endif
	return NULL;
}

/*
	Realise une copie pour eviter qu'une partie du tlv modifie la valeur de retour
*/
uint16_t getPort(TLV tlv) {
	if(getType(tlv) == NEIGHBOUR_NUM) {
		uint16_t port = 0;
		memcpy(&port,tlv + HEAD_SIZE + IP_SIZE,PORT_SIZE);
		return ntohs(port);
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getPort on bad TLV\n");
	#endif
	return 0;
}

DataHash getHash(TLV tlv) {
	if(getType(tlv) == NETWORK_HASH_NUM) {
		return tlv+HEAD_SIZE;
	}
	if(getType(tlv) == NODE_HASH_NUM || getType(tlv) == NODE_STATE_NUM) {
		return tlv + HEAD_SIZE + ID_SIZE + SEQNO_SIZE;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getHash on bad TLV\n");
	#endif
	return NULL;
}

uint64_t getID(TLV tlv) {
	if(getType(tlv) == NODE_STATE_NUM || getType(tlv) == NODE_STATE_REQUEST_NUM || getType(tlv) == NODE_HASH_NUM) {
		uint64_t id = 0;
		memcpy(&id,tlv + HEAD_SIZE,ID_SIZE);
		return id;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getID on bad TLV\n");
	#endif
	return 0;
}

uint16_t getSeqno(TLV tlv) {
	if(getType(tlv) == NODE_HASH_NUM || getType(tlv) == NODE_STATE_NUM) {
		uint16_t seqno = 0;
		memcpy(&seqno,tlv + HEAD_SIZE + ID_SIZE,SEQNO_SIZE);
		return ntohs(seqno);
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getSeqno on bad TLV\n");
	#endif
	return 0;
}

unsigned char* getData(TLV tlv) {
	if(getType(tlv) == NODE_STATE_NUM) {
		return tlv + NODE_STATE_MIN_SIZE;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getData on bad TLV\n");
	#endif
	return NULL;
}

uint8_t getSizeData(TLV tlv) {
	if(getType(tlv) == NODE_STATE_NUM) {
		return size(tlv) - NODE_STATE_MIN_SIZE;
	}
	#ifdef TEST_UNEXCEPTED
		printf("UNEXCEPTED : getSizeData on bad TLV\n");
	#endif
	return 0;
}

uint8_t getLengthTLV(TLV tlv) {
	if(getType(tlv) == PAD1_NUM) {
		return 0;
	}
	uint8_t p = 0;
	memcpy(&p,tlv + TYPE_SIZE,LENGTH_SIZE);
	return p;
}

uint8_t size(TLV tlv) {
	if(getType(tlv) == PAD1_NUM) return 1;
	return HEAD_SIZE + getLengthTLV(tlv);
}

short verifyIP(unsigned char IP[16]) {
	//adresse local au lien (soit unicast, soit multicast)
	return !(IN6_IS_ADDR_LINKLOCAL(IP) || IN6_IS_ADDR_MC_LINKLOCAL(IP));
}

short verify(TLV tlv) {
	int size = getLengthTLV(tlv);

	if(getType(tlv) == PAD1_NUM) {
		return 1;

	}
	else if(getType(tlv) == NEIGHBOUR_REQUEST_NUM || getType(tlv) == NETWORK_STATE_REQUEST_NUM) {
		if(size == NEIGHBOUR_REQUEST_SIZE - HEAD_SIZE) {
			return 1;
		}
		snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size %d for tlv of type %" PRIu8 " but found %" PRIu8 "",NEIGHBOUR_REQUEST_SIZE - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
		return 0;
	}
	else if(getType(tlv) == PADN_NUM) {
		if(size == 0) {
			return 1;
		}
		void* cmp = calloc(size,1);
		if(memcmp(cmp, tlv+HEAD_SIZE, size) == 0) {
			free(cmp);
			return 1;
		}
		free(cmp);
		snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted a sequence of 0 in a tlv of type 1");
		return 0;

	}
	else if(getType(tlv) == NEIGHBOUR_NUM) {
		if(size != NEIGHBOUR_SIZE - HEAD_SIZE) {
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size %d for tlv of type %" PRIu8 " but found %" PRIu8,NEIGHBOUR_SIZE - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
			return 0;
		}
		if(!verifyIP(getIP(tlv)) || IN6_IS_ADDR_LOOPBACK(getIP(tlv))) {
			char IP[40];
			inet_ntop(AF_INET6,getIP(tlv),IP,100);
			#ifdef TEST_IGNORED_ADDRESSES
				printf("IGNORED_ADDRESSES : %s\n",IP);
			#endif
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: avoid local-link address in tlv (%s)",IP);
			return 0;
		}
		//verification IP et Port

	}
	else if(getType(tlv) == NETWORK_HASH_NUM) {
		if(size != NETWORK_HASH_SIZE - HEAD_SIZE) {
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size %d for tlv of type %" PRIu8 "but found %" PRIu8,NETWORK_HASH_SIZE - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
			return 0;
		}
		//verification Hash
	}
	else if(getType(tlv) == NODE_HASH_NUM) {
		if(size != NODE_HASH_SIZE - HEAD_SIZE) {
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size %d for tlv of type %" PRIu8 " but found %" PRIu8,NODE_HASH_SIZE - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
			return 0;
		}
		//verification ID, Seqno, Hash

	}
	else if(getType(tlv) == NODE_STATE_REQUEST_NUM) {
		if(size != NODE_STATE_REQUEST_SIZE - HEAD_SIZE) {
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size %d for tlv of type %" PRIu8 " but found %" PRIu8,NODE_STATE_REQUEST_NUM - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
			return 0;
		}
		//verification ID
	}
	else if(getType(tlv) == NODE_STATE_NUM) {
		if(size < NODE_STATE_MIN_SIZE - HEAD_SIZE) {
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted content size at least %d for tlv of type %" PRIu8 " but found %" PRIu8,NODE_STATE_MIN_SIZE - HEAD_SIZE,getType(tlv),getLengthTLV(tlv));
			return 0;
		}
		Data d = initData(getID(tlv));
		updateData(d,getSeqno(tlv),getData(tlv),getSizeData(tlv));
		DataHash dh = d->hash_data;
		if(memcmp(dh,getHash(tlv),HASH_SIZE) != 0) {
			char excepted[HASH_SIZE*2+1] = {0};
			char found[HASH_SIZE*2+1] = {0};
			for(int i = 0; i < HASH_SIZE; i++) {
				char tmp[3];
				snprintf(tmp,3,"%02x",dh[i]);
				memcpy(excepted+i*2,tmp,2);
			}
			DataHash found_hash = getHash(tlv);
			for(int i = 0; i < HASH_SIZE; i++) {
				char tmp[3];
				snprintf(tmp,3,"%02x",found_hash[i]);
				memcpy(found+i*2,tmp,2);
			}
			snprintf((char*)last_message,MAX_ERROR_SIZE,"-: excepted Hash %s but found %s",excepted,found);
			freeData(d);
			return 0;
		}
		freeData(d);
	
		//verification ID, Seqno, Hash, Data
	}
	else if(getType(tlv) == WARNING_NUM) {
		return 1;
	}
	return 1;
}