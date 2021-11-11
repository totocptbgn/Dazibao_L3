#define _GNU_SOURCE
#define __USE_GNU


#include "inondation.h"


union {
	struct cmsghdr hdr;
	unsigned char buf[CMSG_SPACE(sizeof(struct in6_pktinfo))];
} cmsgbuf;



char status[MAX_DATA_SIZE];

/*
	Initialise le socket du node avec toutes ses options
*/
void initSocket(Node n) {
	int s = socket(PF_INET6, SOCK_DGRAM, 0);
	if(s < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : invalid socket\n");
		#endif
		exit(1);
	}
	int value_0 = 0;
	int value_1 = 1;
	if(setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &value_0,sizeof(value_0)) < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : IPV6_V6ONLY failed\n");
		#endif
		exit(1);
	}
	if(setsockopt(s, IPPROTO_IPV6, IPV6_RECVPKTINFO, &value_1, sizeof(value_1)) < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : IPV6_RECVPKTINFO failed\n");
		#endif
		exit(1);
	}
	
	int rc = fcntl(s, F_GETFL);
	if(rc < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : fcntl F_GETFL failed\n");
		#endif
	}
	rc = fcntl(s, F_SETFL, rc | O_NONBLOCK);
	if(rc < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : fcntl O_NONBLOCK failed\n");
		#endif
	}
	
	n -> socket = s;
}

/*
	Realise le bind du socket
*/
static void bindSocket(Node n,uint16_t port) {
	struct sockaddr_in6 server;
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	server.sin6_port = htons(port);
	int rc = bind(n -> socket, (struct sockaddr*)&server, sizeof(server));
	n -> bind_value = rc;
	if(rc < 0) {
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : bind failed\n");
		#endif
		exit(1);
	}
}

/*
	Envoie un datagramme contenant [paquet] au Voisin [v] depuis le noeud [n]
	Utilise sendmsg
*/
static short sendPackage(Node n, Voisin v,Package paquet) {
	if(isEmpty(paquet) || v == NULL) {
		destroyPackage(paquet);
		return 1;
	}
	int rc;
	struct iovec iov[1];	
	struct sockaddr_in6 server; 
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	
	
	#ifdef TEST_SEND
		char IP[40]; 
		inet_ntop(server.sin6_family, v -> IP,IP, 40);
		printf("SEND : TO %s\n",IP);
	#endif
	memcpy(&server.sin6_addr,v -> IP,sizeof(server.sin6_addr));

	
	server.sin6_port = htons( v -> port);

	int reqlen = getPackageLength(paquet);
	
	void* tosend = build(paquet);

	if(tosend == NULL) { 
		exit(1);//probleme de memoire, on arrete
	}
	again:
	
	iov[0].iov_base = tosend;
	iov[0].iov_len = reqlen;
	struct msghdr msg;
	
	
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (struct sockaddr*) &server;
	msg.msg_namelen = sizeof(server);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	char empty[sizeof(v -> SEND_IP)] = {0};
	if(memcmp(empty ,&(v->SEND_IP),sizeof(v -> SEND_IP)) != 0) {
		struct cmsghdr *cmsg;
		struct in6_pktinfo info;
		memset(&info, 0, sizeof(info));
		info.ipi6_addr = v->SEND_IP;
		memset(cmsgbuf.buf, 0, sizeof(cmsgbuf.buf));
		msg.msg_control = cmsgbuf.buf;
		msg.msg_controllen = sizeof(cmsgbuf.buf);
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = IPPROTO_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
		memcpy(CMSG_DATA(cmsg), &info, sizeof(struct in6_pktinfo));
	}
	rc = sendmsg(n -> socket, &msg, 0);
	if(rc < 0) {
		if(errno == EAGAIN) {
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(n -> socket, &writefds);
			select(n -> socket + 1, NULL, &writefds, NULL, NULL);
			goto again;
		}
		#ifdef TEST_UNEXCEPTED
			printf("UNEXCEPTED : error sendmsg\n");
		#endif
		free(tosend);
		return 0;
	}
	free(tosend);
	return 1;

}

/*

	Réenvoie les données en suspend : TLV avec action SENT dans l'historique

*/

static void reask(Node n) {
	Package paquet;
	HistoryIterator iterator;
	TLV tlv;
	for(int i = 0;i < n -> nbvoisins ;i++) {
		paquet = new_paquet();
		
		iterator = initHistoryIterator(n -> history, -1, SENT,-1, n -> voisins [i]);
		if(iterator == NULL) {
			return;
		}
		while(hasNextTLVMessage(iterator)) {
			tlv = getNextTLVMessage(iterator);
			if(getType(tlv) != NEIGHBOUR_REQUEST_NUM && getType(tlv) != NETWORK_STATE_REQUEST_NUM) {
				if(addTLV(paquet, tlv) < 0) {
					sendPackage(n,  n -> voisins [i],paquet);
					paquet = new_paquet();
					
				}
			}

		}
		free(iterator);
		sendPackage(n,  n -> voisins [i],paquet);
	}
}

/*

	Ensemble des operations executés toutes les 20 secondes

*/
static void every20seconds(Node n) {
	#ifdef TEST_VOISINS
		printVoisins(n);
	#endif	

	#ifdef TEST_DATA
		printDataBase(n->table);
	#endif
	
	if(n->default_value) {
		int nbdata = getNbData(n -> table);
		Data d = initData(n -> id);
		char content[33];
		snprintf(content, 33, "I see %04d data in this database",nbdata);
		if(updateData(d,-1,content,32))
			updateMyData(n -> table,d);
		else free(d);
	}
	reask(n);
	#ifdef TEST_HISTORY
		printHistory(n -> history);
	#endif
	TLV tlv;
	//envoie TLV Network Hash aux amis
	
	Package package;
	DataHash dh = getNetworkHash(n -> table);
	
	for(int i = 0;i < n -> nbvoisins; i++) {
		package = new_paquet();
		tlv = networkHash(dh);
		if(tlv != NULL) 
			addTLV(package,tlv);
		sendPackage(n,n -> voisins[i],package);
		
	}
	removeVoisinsInactifs(n);
	if(n -> nbvoisins < MIN_VOISINS && n -> nbvoisins != 0) {
		
		//envoie TLV Neighbour request a un voisin
		int x = rand() % n -> nbvoisins;
				
		package = new_paquet();
		TLV tlv = neighbourRequest();
		if(tlv != NULL)
			addTLV(package,tlv);
		sendPackage(n,n -> voisins[x],package);
		
		//libère la memoire de l'ancien tlv
		removeInHistory(n -> history, NEIGHBOUR_REQUEST_NUM,SENT,-1,n -> voisins[x],0);
		addInHistory(n -> history, SENT, n -> voisins [x], tlv);
		
	}

}

static Package tryToAddPackage(Node n,Voisin pair,Package answer_pack,TLV tlv_to_send) {
	if(tlv_to_send!=NULL) {
		if(addTLV(answer_pack,tlv_to_send) < 0) {
			sendPackage(n, pair,answer_pack);
			answer_pack = new_paquet();
			if(addTLV(answer_pack,tlv_to_send) < 0) {
				return NULL;
			}
		}
		return answer_pack;
	}
	return NULL;
}

/*
	Ajoute un warning de message [message] au package [answer_pack]
*/
static Package addWarning(Node n,Voisin pair,Package answer_pack,unsigned char* message) {
	#ifdef TEST_WARNING
		char IP[100];
		inet_ntop(AF_INET6,pair -> IP,IP,100);
		printf("WARNING : send Warning to %s :",IP);
		printf("%s\n",message);
	#endif
	TLV tlv_to_send;
	tlv_to_send = warning(message,strlen((char*)message));
	return tryToAddPackage(n,pair,answer_pack,tlv_to_send);
	
}

/*

	Fonctions de traitement des TLV :
	NULL si quelque-chose ne c'est pas passé comme prévu
	sinon le paquet ( qui peut etre different si celui-ci doit être envoyé )

*/

/*
	Reception TLV Pad1
	En réponse à : None
	Envoyé après : None
	Envoi : None
	Nombre d'envoi : None
*/

/*
	Reception TLV Padn
	En réponse à : None
	Envoyé après : None
	Envoi : None
	Nombre d'envoi : None
*/

/*
	Reception TLV
	En réponse à : None
	Envoyé après : None
	Envoi : Neighbour
	Nombre d'envoi : 1
*/

static  Package receiveTLVNeighbourRequest(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	
	TLV tlv_to_send;
	int x = rand() % n -> nbvoisins;
	tlv_to_send = neighbour(n -> voisins [x] -> IP, n -> voisins [x] -> port);
	return tryToAddPackage(n,pair,answer_pack,tlv_to_send);
}

/*
	Reception TLV Neighbour
	En réponse à : NeighbourRequest
	Envoyé après : None
	Envoi : Hash Network
	Nombre d'envoi : 1
*/


static  Package receiveTLVNeighbour(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	TLV tlv_to_send;
	if(searchInHistory(n -> history, NEIGHBOUR_REQUEST_NUM,SENT,-1,pair)) {	
		if(isVoisin(n,getPort(tlv), getIP(tlv),0) == NULL) {	
			Package p = new_paquet();
			DataHash h =getNetworkHash(n -> table);
			tlv_to_send = networkHash(h);
			if(tlv_to_send != NULL) {
				if(addTLV(p,tlv_to_send) == 0) {
					Voisin nouveau = malloc(sizeof(struct voisin));
					if(nouveau == NULL) {
						#ifdef TEST_MEMORY
							printf("MEMORY : malloc voison of receiveTLVNeighbour failed\n");
						#endif
						return NULL;
					}
					memset(nouveau,0,sizeof(struct voisin));
					nouveau -> port = getPort(tlv);	
					memcpy(nouveau -> IP,getIP(tlv),IP_SIZE);
					sendPackage(n,nouveau,p);
					free(nouveau);
					return answer_pack;
				}	
			}
		}
		removeInHistory(n -> history ,NEIGHBOUR_REQUEST_NUM, SENT,-1, pair, 0);
	}
	return NULL;
}

/*
	Reception TLV Network Hash
	En réponse à : None
	Envoyé après : None
	Envoi : Network State Request
	Nombre d'envoi : [0-1]
*/
static  Package receiveTLVNetworkHash(Node n,Voisin pair,Package answer_pack,TLV tlv) {

	TLV tlv_to_send;
	DataHash network_hash = getHash(tlv);
	DataHash h = getNetworkHash(n -> table);
	if(memcmp(network_hash,h,HASH_SIZE) != 0) {
		#ifdef TEST_HASH_COMPARE
			printf("HASH COMPARE : ");
			for(int i = 0;i<HASH_SIZE;i++) {
				printf("%02x",h[i]);
			}
			printf("-");
			for(int i = 0;i<HASH_SIZE;i++) {
				printf("%02x",network_hash[i]);
			}
			printf("are differents\n");
		#endif
		tlv_to_send = networkStateRequest();
		removeInHistory(n -> history,NETWORK_STATE_REQUEST_NUM , SENT, -1,pair,0);
		addInHistory(n -> history, SENT, pair, tlv_to_send);
		return tryToAddPackage(n,pair,answer_pack,tlv_to_send);
	}
	else {
		#ifdef TEST_HASH_COMPARE
			printf("HASH COMPARE : ");
			for(int i = 0;i<HASH_SIZE;i++) {
				printf("%02x",h[i]);
			}
			printf("-");
			for(int i = 0;i<HASH_SIZE;i++) {
				printf("%02x",network_hash[i]);
			}
			printf("are same\n");
		#endif
	}
	return NULL;
}


/*
	Reception TLV Network State Request
	En réponse à : Network Hash
	Envoyé après : None
	Envoi : Node Hash
	Nombre d'envoi : [0-nbdata]
*/

static  Package receiveTLVNetworkStateRequest(Node n,Voisin pair,Package answer_pack,TLV tlv) {

	TLV tlv_to_send;
	Package new_package;
	DataIterator iterator = initDataIterator(n -> table);	
	if(iterator == NULL) return NULL;
	Data d;
	while(hasNextData(iterator)) {
		d = getNextData(iterator);
		DataHash node_hash = d -> hash_data;
			
		if(node_hash == NULL) continue;
		tlv_to_send = nodeHash(d -> id, d -> seqno , node_hash);
		new_package = tryToAddPackage(n,pair,answer_pack,tlv_to_send);
		if(new_package != NULL) {
			answer_pack = new_package;
		}
		
	}
	free(iterator);
	return answer_pack;
}

/*
	Reception TLV Node Hash
	En réponse à : Network State Request
	Envoyé après : Network Hash
	Envoi : Node State Request ou Node State
	Nombre d'envoi : [0-1]
*/
static  Package receiveTLVNodeHash(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	TLV tlv_to_send;
	if(searchInHistory(n -> history,NETWORK_STATE_REQUEST_NUM,SENT,-1,pair)) {
		
		DataHash data_hash1 = getHash(tlv);
		uint64_t id = getID(tlv);
		uint16_t seqno = getSeqno(tlv);
		DataHash data_hash2 = NULL;
		Data d2 = getDataBase(n -> table,id);
		if(d2 != NULL) {
			data_hash2 = d2 -> hash_data;
		}
		//si id de la base > id du tlv : envoie node State
		if(d2 != NULL && d2 -> seqno !=  seqno && ((d2 -> seqno - seqno) & 32768) == 0) {
			
			#ifdef TEST_SPECIAL_DELIVERY
				printf("--- SPECIAL DELIVERY ---\n");
				char IP[100];
				inet_ntop(AF_INET6,pair -> IP,IP,100);
				printf("With %s : \n",IP);
				printf("Node id : " PrID " (my seqno : %d his seqno : %d)\n",PrID_value(id) ,d2 -> seqno, seqno);
				printf("--- ---------------- ---")
			#endif
			tlv_to_send = nodeState(d2 -> id, d2 -> seqno, d2 -> hash_data , d2 -> content,d2 -> size);
			return tryToAddPackage(n,pair,answer_pack,tlv_to_send);
		}
		else if(d2 == NULL || d2 -> seqno !=  seqno || memcmp(data_hash1,data_hash2,HASH_SIZE) != 0) {
			tlv_to_send = nodeStateRequest(getID(tlv));
			if(addTLV(answer_pack,tlv_to_send) < 0) {
				sendPackage(n, pair,answer_pack);
				answer_pack = new_paquet();
				if(addTLV(answer_pack,tlv_to_send) <0)
					return NULL;
			}
			removeInHistory(n -> history,NODE_STATE_REQUEST_NUM,SENT,getID(tlv),pair,0);	
			addInHistory(n -> history, SENT, pair, tlv_to_send);
			return answer_pack;
			
		}
		
	}
	return NULL;
}

/*
	Reception TLV Node State Request
	En réponse à : Node Hash
	Envoyé après :  Network State Request
	Envoi : Node State
	Nombre d'envoi : [0-1]
*/
static  Package receiveTLVNodeStateRequest(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	TLV tlv_to_send;

	//envoie Node State
	Data d = getDataBase(n -> table,getID(tlv));
	if(d == NULL) return NULL;
	DataHash h = d -> hash_data;
	tlv_to_send = nodeState(d -> id, d -> seqno, h , d -> content,d -> size);
	return tryToAddPackage(n,pair,answer_pack,tlv_to_send);
}


/*
	Reception TLV Node State
	En réponse à : Node State Request
	Envoyé après : Node Hash
	Envoi : None
	Nombre d'envoi : None
*/

static  Package receiveTLVNodeState(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	//si TLV Node State Request correspondant dans l'historique
	if(searchInHistory(n -> history, NODE_STATE_REQUEST_NUM,SENT,getID(tlv), pair)) {
		removeInHistory(n -> history,NODE_STATE_REQUEST_NUM, SENT,getID(tlv), pair, 0);
		//change donnée en consequence
		Data d = initData(getID(tlv));
		if(d == NULL) return NULL;
		if(!updateData(d,getSeqno(tlv),getData(tlv),getSizeData(tlv))) {
			free(d);
			return NULL;
		}
		updateDataBase(n -> table, d, n -> id);
	
		return answer_pack;
	}
	return NULL;
}

/*
	Reception TLV Warning
	En réponse à : None
	Envoyé après : None
	Envoi : None
	Nombre d'envoi : None
*/

static  Package receiveTLVWarning(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	/*
		Stockage de l'erreur dans un fichier
	*/
	int fd = open(LOG_FILE,O_APPEND | O_WRONLY | O_CREAT,S_IRWXU);
	char IP[40] = {0};
	inet_ntop(AF_INET6,pair -> IP,IP,40);
	write(fd,IP,strlen(IP));
	write(fd," ",1);
	write(fd,getMessage(tlv),getLengthTLV(tlv));
	write(fd,"\n",1);
	close(fd);
	return answer_pack;
}

/*
	Reception TLV Change Message
	En réponse à : None
	Envoyé après : None
	Envoi : None
	Nombre d'envoi : None
*/

static Package receiveTLVChangeMessage(Node n,Voisin pair,Package answer_pack,TLV tlv) {
	
	Data d = initData(n -> id);
	void * buffer = getMessage(tlv);
	int buffer_size = getLengthTLV(tlv);

	char* decrypted_message = decrypt_buffer(buffer, buffer_size);
	if (decrypted_message == NULL) {
		return NULL;
	}

	if(!updateData(d,-1, decrypted_message, strlen(decrypted_message))) return NULL;
	updateMyData(n -> table,d);
	free(decrypted_message);
	generate_keys();

	
	return answer_pack;
}

/*
	Reception TLV Stock
	En réponse à : None
	Envoyé après : None
	Envoi : Pad1
	Nombre d'envoi : None
*/

static Package receiveTLVStock(Node n,Voisin pair,Package answer_pack,TLV tlv) {	
	
	stockDatabase(n->table);
	struct voisin v;
	memset(&v,0,sizeof(v));
	inet_pton(AF_INET6,"::1",v.IP);
	v.port = CHANGE_PORT;
	TLV tlv_to_send = pad1();
	Package p = new_paquet();
	addTLV(p,tlv_to_send);
	sendPackage(n,&v,p);
	return answer_pack;
	
	
}
/*
	Initialisation et Boucle d'evenemement
*/

void startCommunication(Node n,uint16_t port) {
	srand(time(NULL));
	struct msghdr msg;
	struct iovec iov[1];

	unsigned char* buffer[MAX_PACKAGE_SIZE];
	
	Package receive_pack;
	Package answer_pack;
	
	TLV receive_tlv;
	
	Voisin pair;
		
	initSocket(n);
	bindSocket(n,port);
	/*
		Boucle evenements
	*/
	time_t last_send = time(NULL);
	#ifdef TEST_DEBUG
		printf("--- Inondation start ---\n");
	#endif
	every20seconds(n);

	while(1) {
			
			
		if( difftime( time(NULL), last_send ) >= 20 ) {	
			last_send = time(NULL);
			every20seconds(n);
		}			
			
		struct sockaddr_in6 client;
		socklen_t client_len = sizeof(client);
		
		int seconds = 1; //temps d'attente de la reception du message
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(n -> socket, &readfds);
		struct timeval tv = {seconds, 0};
		int rc = select(n -> socket + 1, &readfds, NULL, NULL, &tv);
		if(rc < 0) {
			#ifdef TEST_UNEXCEPTED
				printf("UNEXCEPTED : error select\n");
			#endif
			continue;
		}
		if(rc > 0) {
			if(FD_ISSET(n -> socket, &readfds)) {
				memset(&cmsgbuf,0,sizeof(cmsgbuf));
				memset(&msg,0,sizeof(msg));
				struct sockaddr_in6 addr;
				iov[0].iov_base = buffer;
				iov[0].iov_len = MAX_PACKAGE_SIZE;
				msg.msg_name = &addr;
				msg.msg_namelen = sizeof(addr);
				msg.msg_iov = iov;
				msg.msg_iovlen = 1;	
				msg.msg_control = (struct cmsghdr*)cmsgbuf.buf;
				msg.msg_controllen = sizeof(cmsgbuf.buf);
				rc = recvmsg(n->socket,&msg,0);
			}
			else {
				#ifdef TEST_UNEXCEPTED
					printf("UNEXCEPTED : FD_ISSET failed but select did not\n");
				#endif
				continue;
			}
			
			if(rc < 0) {
				if(errno != EAGAIN) {
					#ifdef TEST_UNEXCEPTED
						printf("UNEXCEPTED : recvmsg failed\n");
					#endif
				}
				else {
					#ifdef TEST_UNEXCEPTED
						printf("UNEXCEPTED : recvfrom is EAGAIN\n");
					#endif
				}		
			}
			else if(rc > 0) {
				unsigned char addr[16];
				uint16_t port;

				/*
					recuperation champs addresse et port
				*/
				#ifdef TEST_RECEIVE
					char IP_char[40];
					inet_ntop (AF_INET6, &client.sin6_addr, IP_char, 40);
					printf("RECEIVE : FROM :\n%s\n",IP_char);
				#endif
					
				memcpy(&client,msg.msg_name,msg.msg_namelen);
				memcpy(addr,&(client.sin6_addr),IP_SIZE);
				port = htons(client.sin6_port);
					
				if(client_len != sizeof(client) || client.sin6_family != AF_INET6) {
					continue;
				}				
				pair = NULL;
				if((pair = isVoisin(n,port,addr,1)) == NULL && !mySelf(n,port,addr)) {
					//l'ajoute en ami
					pair = addVoisin(n,port,addr);
					if(pair == NULL) { //en cas d'echec
						continue;
					}
				}	
				updateReceiveWith(msg,pair);
				answer_pack = new_paquet();
				
				receive_pack = paquet(buffer, rc, answer_pack);
				if(receive_pack == NULL) {
					unsigned char* error = getPackageConstructionError();
					if(error != NULL) addWarning(n,pair,answer_pack,error);
					sendPackage(n, pair,answer_pack);
					continue;
				}
					
				while((receive_tlv = getNextTLV(receive_pack)) != NULL) {
					if(getType(receive_tlv) == CHANGE_MESSAGE_NUM) {
						Package p = receiveTLVChangeMessage(n,pair,answer_pack,receive_tlv);
						if(p != NULL) answer_pack = p;
					}
					else if(pair == NULL && getType(receive_tlv) == STOCK_NUM) {
						Package p = receiveTLVStock(n,pair,answer_pack,receive_tlv);
						if(p != NULL) answer_pack = p;
					}
					else if(pair != NULL) {
						if(getType(receive_tlv) == NEIGHBOUR_REQUEST_NUM)  {
							Package p = receiveTLVNeighbourRequest(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
						}
						else if(getType(receive_tlv) == NEIGHBOUR_NUM)  {
							Package p = receiveTLVNeighbour(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
								
						}
						else if(getType(receive_tlv) == NETWORK_HASH_NUM)  {
							Package p = receiveTLVNetworkHash(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
						}
						else if(getType(receive_tlv) == NETWORK_STATE_REQUEST_NUM)  {
							Package p = receiveTLVNetworkStateRequest(n,pair,answer_pack,receive_tlv);
								if(p != NULL) answer_pack = p;	
						}
						else if(getType(receive_tlv) == NODE_HASH_NUM)  {
							Package p = receiveTLVNodeHash(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
						}
						else if(getType(receive_tlv) == NODE_STATE_REQUEST_NUM)  {
							Package p = receiveTLVNodeStateRequest(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
						}
						else if(getType(receive_tlv) == NODE_STATE_NUM) {
							Package p = receiveTLVNodeState(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;	
						}
						else if(getType(receive_tlv) == WARNING_NUM) {
							Package p = receiveTLVWarning(n,pair,answer_pack,receive_tlv);
							if(p != NULL) answer_pack = p;
						}
					}
				}
				sendPackage(n,pair,answer_pack);
				destroyPackage(receive_pack);
			}
		}
	}
}