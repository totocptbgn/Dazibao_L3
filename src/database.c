#include "database.h"


struct database {
	Data* content;
	int nbdata;
	unsigned char network_hash[HASH_SIZE];
};

//realise le hash de la database [base] et le renvoie
static DataHash hashNetwork(DataBase base);

//réalise le hash de la donnée [data] et le renvoie
static DataHash hash(Data data);

#ifdef TEST_DATA
	void printDataBase(DataBase db) {
		printf("--- DataBase ---\n");
		printf("Network Hash : ");
		for(int j = 0;j < HASH_SIZE;j++) {
			printf("%02x",db -> network_hash[j]);
		}
		printf("\n");
		printf("- Size : %d\n",db -> nbdata);
		
		for(int i = 0; i < db -> nbdata; i++) {
			printf("- Data : \n");
			printf("\tID : " PrID "\n",PrID_value(db ->content[i] ->id));
			printf("\tSeqno : %"PRIu16"\n\t",db -> content[i] -> seqno);
			fflush(stdout);
			write(1,db -> content[i] -> content,db -> content[i] -> size);
			printf("\n");
			printf("\tNode Hash : ");
			for(int j = 0;j < HASH_SIZE;j++) {
				printf("%02x",db ->content[i] ->hash_data[j]);
			}
			printf("\n");
		}
		printf("--- -------- ---\n");
		
	}
#endif

int getNbData(DataBase db) {
	return db -> nbdata;
}
Data initData(uint64_t id) {
	Data d = malloc(sizeof(struct data));
	memset(d,0,sizeof(struct data));
	if(d == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc data failed\n");
		#endif
		return NULL;
	}
	d -> seqno = -1;
	d -> id = id;
	d -> content = NULL;
	return d;
}

short updateData(Data d,uint16_t seqno,void* content,int size) {
	short change = 0;
	if(d -> seqno == -1) change = 1;
	if(seqno != -1 && d->seqno!=seqno) {
		d->seqno = seqno;
		change = 1;
	}
	if(size >= 0 && (size!=d->size || memcmp(content,d->content,size) != 0)) {
		free(d->content);
		d->content = malloc(size);
		if(size!=0)
			memcpy(d->content,content,size);
		d->size = size;
		change = 1;
	}
	if(change) {
		DataHash dh = hash(d);
		if(dh == NULL) {
			free(d->content);
			return 0;
		}
		memcpy(d->hash_data,dh,HASH_SIZE);
		free(dh);
	
	}
	return change;
	
	
}

void freeData(Data d) {
	free(d -> content);
	free(d);
}

DataBase initDataBase(Data d) {
	DataBase db = malloc(sizeof(struct database));
	if( db == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc database failed\n");
		#endif
		return NULL;
	}
	db -> nbdata = 1;
	db -> content = malloc(sizeof(Data));
	if( db -> content == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc database content failed\n");
		#endif
		return NULL;
	}
	db -> content[0] = d;
	DataHash hash_network= hashNetwork(db);
	if(hash_network == NULL) {
		free(db -> content);
		return NULL;
	}
	memcpy(db -> network_hash,hash_network,HASH_SIZE);
	free(hash_network);
	return db;
}

short updateDataBase(DataBase base, Data d,uint64_t myid) {
	int after = -1;
	for(int i = 0; i < base -> nbdata; i++) {
		if(base -> content[i] -> id == d -> id) {
			if(d -> id == myid) {
					if(((d -> seqno - base -> content[i] -> seqno) & 32768) == 0) {
						if(!updateData(base -> content[i],(d -> seqno + 1) & 65535, NULL, -1)) {
							freeData(d);
							return -1;
						}
						DataHash dh= hashNetwork(base);
						if(dh == NULL) {
							freeData(d);
							return -1;
						}
						memcpy(base -> network_hash,dh,HASH_SIZE);
						free(dh);
					}
					freeData(d);
					return 0;
			}
			else {

				if(((base -> content[i] -> seqno - d -> seqno) & 32768) == 0) {
					freeData(d);
					return 0;
				}
				else {
					Data d_prev = base -> content [i];
					
					base -> content [i] = d;
					DataHash dh= hashNetwork(base);
					if(dh==NULL) {
						base -> content [i] = d_prev;
						freeData(d);
						return -1;
					} 
					freeData(d_prev);
					memcpy(base -> network_hash,dh,HASH_SIZE);
					free(dh);
				}
			}
			#ifdef TEST_DATA
				printf("DATA : update 1 Data\n");		
			#endif
			return 1;
		}
		else if(after == -1 && base -> content[i] -> id > d -> id) {
			after = i;
		}
	}
	void* newpointeur;
	if((newpointeur = realloc(base -> content, sizeof(Data)*( 1 + base -> nbdata))) != NULL) {
		base -> content = newpointeur;
		if(after != -1)
			memmove(base -> content + after + 1,base -> content + after, (base -> nbdata - after) * sizeof(Data));
		else 
			after = base -> nbdata;
		base -> content [ after ] = d;
		base -> nbdata ++;
		DataHash dh= hashNetwork(base);
		memcpy(base -> network_hash,dh,HASH_SIZE);
		free(dh);
		#ifdef TEST_DATA
			printf("DATA : add 1 Data\n");
			printf("DATA : database size : %d\n",base -> nbdata);	
		#endif
		
		
		return 1;
	}
	else {
		#ifdef TEST_MEMORY
			printf("MEMORY : realloc database content failed\n");
		#endif
		freeData(d);
		return 0;
	}
}	
Data getDataBase(DataBase db,uint64_t id) {
	for(int i = 0; i < db -> nbdata; i++) {
		if(db -> content [i] -> id == id) {		
			return db -> content[i];
		}
	}
	return NULL;
}	

DataHash getNetworkHash(DataBase dh) {
	return dh->network_hash;
}

void stockDatabase(DataBase db) { 
	
	if(fork() == 0) {
		int fd = open(DATA_FILE,O_WRONLY | O_CREAT | O_TRUNC,S_IRWXU);
		if(fd <= 0) {
			exit(0);
		}
		if(flock(fd, LOCK_EX) != 0) {
			close(fd);
			exit(0);
		}
		for(int i = 0; i < db -> nbdata; i++) {
			char ID_char[17];
			snprintf(ID_char,17,PrID,PrID_value(db -> content[i] -> id));
			write(fd,ID_char,16);
			write(fd," ",1);
			int size =  db -> content[i] -> size;
			char hexa[3];
			for(int j = 0;j < size; j++) {
				snprintf(hexa,3,"%02x",((char*) db -> content[i] -> content)[j]);
				write(fd,hexa,2);
			}
			if(i != db->nbdata - 1)
				write(fd,"\n",1);
		}
		flock(fd,LOCK_UN);
		close(fd);
		exit(0);
	}
}

short updateMyData(DataBase db,Data d) {
	
	for(int i = 0; i < db -> nbdata; i++) {
		if(db -> content [i] -> id == d -> id && (d -> size != db -> content[i] -> size 
			|| memcmp(d -> content, db -> content[i] -> content, d -> size) != 0)) {
			Data d_prev = db -> content [i];
			updateData(d,(d_prev -> seqno + 1) & 65535,NULL,-1);
			db -> content[i] = d;
			DataHash dh= hashNetwork(db);
			if(dh==NULL) {
				db -> content[i] = d_prev;
				return -1;
			} 
			memcpy(db -> network_hash,dh,HASH_SIZE);
			freeData(d_prev);
			free(dh);
			return 1;
			
		}
	}
	freeData(d);
	return 0;
}

/*
	Iterateur de data
*/
struct dataIterator {
	DataBase base;
	int current;
};

DataIterator initDataIterator(DataBase d) {
	DataIterator di = malloc(sizeof(struct dataIterator));
	if(di == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc data iterator failed\n");
		#endif
		return NULL;
	}
	di -> base = d;
	di -> current = 0;
	return di;
}

Data getNextData(DataIterator di) {
	Data d = di -> base -> content [di -> current];
	di -> current++;
	return d;
}

short hasNextData(DataIterator di) {
	return di -> current < di -> base -> nbdata;
}

static DataHash hash(Data dt) {
	#ifdef TEST_CALCUL_HASH
		printf("CALCUL HASH : Hash of " PrID "\n",PrID_value(dt ->id));
	#endif
	unsigned char s[ID_SIZE + SEQNO_SIZE + dt -> size];
	memset(s,0,ID_SIZE + SEQNO_SIZE + dt -> size);
	uint64_t id = dt -> id;
	uint16_t seqno = htons(dt -> seqno);
	memcpy(s,&id,ID_SIZE);
	memcpy(s+ID_SIZE,&seqno,SEQNO_SIZE);
	if(dt -> size != 0)
		memcpy(s + SEQNO_SIZE + ID_SIZE,dt -> content,dt -> size);
    unsigned char *d = SHA256(s,ID_SIZE + SEQNO_SIZE + dt -> size, NULL);
	DataHash res = malloc(HASH_SIZE);
	if(res == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc hash failed\n");
		#endif
		return NULL;
	}
	memcpy(res,d,HASH_SIZE);
	return res;
}


static DataHash hashNetwork(DataBase base) {
	#ifdef TEST_CALCUL_HASH
		printf("CALCUL HASH : Network Hash\n");
	#endif
	unsigned char everyhash[HASH_SIZE * getNbData(base)];

	DataIterator iterator = initDataIterator(base);
	
	for(int i = 0; i < getNbData(base); i++) {
		DataHash dh = getNextData(iterator)->hash_data;

		memcpy(everyhash + i*HASH_SIZE,dh , HASH_SIZE);
	}
	
	free(iterator);
	
	unsigned char *hash = SHA256(everyhash,HASH_SIZE * getNbData(base), NULL);
	
	DataHash finalhash = malloc(HASH_SIZE);

	if(finalhash == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc network hash failed\n");
		#endif
		return NULL;
	}
	memcpy(finalhash,hash,HASH_SIZE);
	return finalhash;
}


