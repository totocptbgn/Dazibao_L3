#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include "node.h"


struct starting_neighbour { //for permanent neighbours
	uint16_t port;
	char* interface;
	struct starting_neighbour* next;
};

struct starting_neighbour* permanents = NULL;
struct starting_neighbour* last = NULL;
void freePermanents() {
	struct starting_neighbour* tmp;
	while(permanents != NULL) {
		tmp = permanents->next;
		free(permanents);
		permanents = tmp;
	}
	last = NULL;
}

short addPermanent(char* interface,uint16_t port) {
	struct starting_neighbour* new_neighbour = malloc(sizeof(struct starting_neighbour));
	if(new_neighbour==NULL) return 0;
	new_neighbour->port=port;
	new_neighbour->interface=interface;
	new_neighbour->next=NULL;
	if(permanents==NULL) {
		permanents = new_neighbour;
		last = new_neighbour;
	}
	else {
		last->next=new_neighbour;
		last = last->next;
	}
	return 1;
}

/*
	Les premiers voisins du noeud : renvoie le nombre ajouté
*/
static int initVoisins(Node n) {
	void* ptr;
	int number = 0;
	struct addrinfo hints;
	struct starting_neighbour* addr = permanents;
	while(addr != NULL) {
		memset(&hints, 0, sizeof(hints));
	
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = 0;
		hints.ai_flags =  AI_V4MAPPED | AI_ALL;
	
		struct addrinfo *res;
	
		uint16_t nport = addr->port;
		char port[16] = {0};
    	snprintf(port, 16, "%d", nport);

		int rc = getaddrinfo(addr->interface, port, &hints, &res);
	 
		if(rc != 0) {
			#ifdef TEST_UNEXCEPTED
				printf("UNEXCEPTED : getaddrinfo failed\n");
				fflush(stdout);
				write(1,gai_strerror(rc),strlen(gai_strerror(rc)));
			#endif
			continue;
		}

		struct addrinfo *p;
		for(p = res; p != NULL; p = p->ai_next) {
        	switch (p->ai_family) {
            	case AF_INET6:
                	ptr = &((struct sockaddr_in6 *) p ->ai_addr)->sin6_addr;
                	break;
			
				case AF_INET:
					ptr = &((struct sockaddr_in6 *) p ->ai_addr)->sin6_addr;
					break;

				default:
					continue;
    	    }
		
			Voisin v = addVoisin(n,addr->port, ptr);
			if(v != NULL) {
				v -> permanent = 1;
				number++;
			}
		
		}
		freeaddrinfo(res);
		addr=addr->next;
	}
	#ifdef TEST_VOISINS
		printf("VOISINS : number of permanent neighbours : %d\n",n -> nbvoisins);
	#endif
	if(number == 0) {
		fprintf(stderr,"Aucun voisin trouvé, utilisé l'option -n si cela est normal\n");
		exit(1);
	}
	return number;
}

/*
	Initialise l'id aléatoirement
*/

static uint64_t initId() {
	srand(time(NULL));
	return rand() % ULLONG_MAX + 1;
}



Node initNode(uint16_t port, uint16_t seqno, uint64_t id_node,short opt_d,short opt_n) {
	Node n = malloc(sizeof(struct node));
	if(n == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc node failed\n");
		#endif
		return NULL;
	}
	memset( n, 0, sizeof(struct node));

	struct STUNServer servers[NB_SERVERS] = SERVERS;
    for (int i = 0; i < NB_SERVERS; i++) {
		if(getPublicIPv4Address(servers[i],n->IPv4addr) == 0) break;
	}
	#ifdef TEST_IGNORED_ADDRESSES
		char empty[16] = {0};
		if(memcmp(empty,n->IPv4addr,16) == 0)
			printf("IGNORED_ADRESSES : no IPv4 found\n");
		else {
			char IP[40];
			inet_ntop(AF_INET6,n->IPv4addr,IP,40);
			printf("IGNORED_ADRESSES : IPv4 found %s\n",IP);
		}
	#endif
	n -> default_value = opt_d;
	n -> neighbours = !opt_n;
	if((n -> history = initHistory()) == NULL)
		return NULL;
	if(id_node == -1)
		n -> id = initId();
	else 
		n -> id = id_node;
	n -> port = port;
	Data d = initData(n -> id);
	if(d == NULL) {
		return NULL;
	}
	
	if(!updateData(d,seqno,"",0)) {
		return NULL;
	}
	
	
	if((n -> table = initDataBase(d)) == NULL) {
		return NULL;
	}

	if(n->neighbours) {
		if(initVoisins(n) == 0) {
			#ifdef TEST_UNEXCEPTED
				printf("UNEXCEPTED : no voisin found\n");
			#endif
			return NULL;
		}
	}
	return n;
}


#ifdef TEST_VOISINS
void printVoisins(Node n) {
	printf("--- VOISINS ---\n");
	char IP[100];
	for(int i = 0; i < n ->nbvoisins; i ++) {
		memset(IP,'\0',100);
		
		inet_ntop(AF_INET6,n ->voisins[i] -> IP,IP,100);
		printf("- %s : %"PRIu16"\n",IP,n ->voisins[i]->port);
	}
	printf("--- ------- ---\n");
}
#endif

short mySelf(Node n, uint16_t port, unsigned char* IP) {
	if(port != n -> port && port!=CHANGE_PORT) return 0;
	if(IN6_IS_ADDR_LOOPBACK(IP)) return 1;
	struct ifaddrs * addrs;
	getifaddrs(&addrs);
	struct ifaddrs* tmp = addrs;
	char empty[IP_SIZE] = {0};
	if(memcmp(n->IPv4addr,empty,IP_SIZE) != 0 && memcmp(n->IPv4addr,IP,IP_SIZE) == 0) {
		#ifdef TEST_IGNORED_ADDRESSES
			char my_IP[40];
			inet_ntop(AF_INET6,(char*)IP,my_IP,40);
			printf("IGNORED_ADDRESSES : %s\n",my_IP);
		#endif
		return 1;
	}
	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 *pAddr = (struct sockaddr_in6 *)tmp->ifa_addr;
			struct in6_addr addressIPv6 = pAddr->sin6_addr;
			if(memcmp(IP,&addressIPv6,IP_SIZE) == 0) {
				freeifaddrs(addrs);
				#ifdef TEST_IGNORED_ADDRESSES
					char my_IP[40];
					inet_ntop(AF_INET6,(char*)IP,my_IP,40);
					printf("IGNORED_ADDRESSES : %s\n",my_IP);
				#endif
				return 1;
			}
		}
		
		
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return 0;	
}
/*

	Gestion voisins
	
*/

Voisin addVoisin(Node n,  uint16_t port, unsigned char* IP) {
	if( n -> nbvoisins == MAX_VOISINS || !verifyIP(IP)) {
		return NULL;
	}
	if(mySelf(n, port, IP)) {
		return NULL;
	}
	
	for(int i = 0;i < n -> nbvoisins;i++) {
		if(n -> voisins[i] -> port == port && memcmp(n -> voisins[i] -> IP,IP,16) == 0) {
			return NULL;
		}
	}
	
	Voisin v = malloc(sizeof(struct voisin));

	if(v == NULL) {
		#ifdef TEST_MEMORY
			printf("MEMORY : malloc voisin failed\n");
		#endif
		return NULL;
	}
	
	
	memset(&(v -> SEND_IP),0,sizeof(v -> SEND_IP));

	v -> port = port;
	memcpy(v -> IP,IP,IP_SIZE);

	v -> permanent = 0;
	v -> last_message = time(NULL);
	n -> voisins [n -> nbvoisins] = v;
	n -> nbvoisins ++;
	#ifdef TEST_VOISINS
		printf("VOISINS : one more neighbour\n");
		printf("VOISINS : number of neighbour : %d\n",n -> nbvoisins);
	#endif
	return v;
}

/*

	Verifie que l'on connait le noeud -> si on le connait met à jour la date du dernier packet recue (si update)

*/
Voisin isVoisin(Node n,uint16_t port, unsigned char* IP,short update) {
	for(int i = 0; i < n -> nbvoisins ;i++) {
		if(n -> voisins[i] -> port == port && memcmp(n -> voisins[i] -> IP,IP,IP_SIZE) == 0) {
			if(update) n -> voisins [i] -> last_message = time(NULL);
			return n -> voisins [i];
		}
	}
	return NULL;
}

/*
	Libere la memoire alloué du voisin
*/
static void freeVoisin(Voisin v) {
	free(v);
}
/*
	supprime le noeud des amis qui n'ont pas communiqué depuis 70 secondes et tous les messages associés dans l'historique
*/
int removeVoisinsInactifs(Node n) {
	int number = 0;
	time_t current = time(NULL);
	for(int i = 0; i < n -> nbvoisins ;i++) {
	
		if(!(n -> voisins[i] -> permanent) && difftime( current, n -> voisins[i] -> last_message) >= 70) {
			removeInHistory(n -> history, -1, -1,-1, n -> voisins[i], 1);
			freeVoisin(n -> voisins[i]);
			n -> voisins[i] = n -> voisins[n -> nbvoisins - 1];
			n -> nbvoisins --;
			number++;
		}
	}
	#ifdef TEST_VOISINS
		printf("VOISINS : number of neighbour : %d\n",n -> nbvoisins);
	#endif
	return number;
}

short cmpVoisins(Voisin a, Voisin b) {
	return memcmp(a -> IP,b -> IP,16) == 0 && a -> port == b -> port;
}

short updateReceiveWith(struct msghdr msg,Voisin v) {
	if(v == NULL) return 0;
	struct cmsghdr *cmsg;
	struct in6_pktinfo *info = NULL;
	cmsg = CMSG_FIRSTHDR(&msg);
	while(cmsg != NULL) {
		if ((cmsg->cmsg_level == IPPROTO_IPV6) && (cmsg->cmsg_type == IPV6_PKTINFO)) {
			info = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			break;
		}
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}
	if(info == NULL) {
		
		#ifdef TEST_DEBUG
			char IP[40];
			inet_ntop(AF_INET6,v -> IP,IP,100);
			printf("DEBUG : no IPV6_PKTINFO with %s\n",IP);
		#endif
		
		return 0;
	}
	memcpy(&v -> SEND_IP, &(info->ipi6_addr),sizeof(info->ipi6_addr));
	return 1;
}