#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include "tlv.h"
#include "package.h"
#include "constants.h"



/*
    Envoie un TLV de type 42 pour change le message
*/

int initSocket() {
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
	return s;
}


int main(int argc, char** argv) {
    
    if ((argc != 2 && argc != 3) || strcmp(argv[1],"--help") == 0) {
        printf("Usage: dazibao_read PORT {ID}\n");
		printf("Ask for the node on this computer on the port [PORT].\n");
		printf("If an ID is given, only give the data of the node with the [ID] ID (should be hexadecimal value)\n");
        printf("Should be executed on the same repertory of the node creation\n");
        exit(1);
    }    

    char* ID = NULL;
    if(argc == 3) ID = argv[2];

	uint16_t port = atoi(argv[1]);

	srand(time(NULL));

	// Ouverture Socket
	int s = initSocket();
	if (s < 0) {
		perror("socket");
		exit(1);
	}

    struct sockaddr_in6 server;
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	server.sin6_port = htons(CHANGE_PORT);

	int rc = bind(s, (struct sockaddr*)&server, sizeof(server));
	if(rc < 0) {
		perror("bind");
		exit(1);
	}

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;

	rc = inet_pton(AF_INET6, "::1", &sin6.sin6_addr);
	if(rc < 1) {
		perror("inet_pton");
		close(s);
		exit(1);
	}
	sin6.sin6_port = htons(port);

    TLV mess = stock();
    if (mess == NULL) {
			close(s);
			exit(1);
	}
    Package paquet = new_paquet();
    int adt = addTLV(paquet, mess);
	
    
	if (adt < 0) {
        if (adt == -1) fprintf(stderr,"addTLV: Plus de place");
        if (adt == -2) fprintf(stderr,"addTLV: Erreur de realloc()");
        close(s);
        exit(1);
    }

    int size = getPackageLength(paquet);
    void* ready_to_send = build(paquet);

    int st = sendto(s, ready_to_send, size, 0, (struct sockaddr*)&sin6, sizeof(sin6));
    if (st < 0) {
        perror("sendto");
        close(s);
        exit(1);
    } else {
        printf("TLV Type 51 envoyé\n");
    }

    struct timeval tv = {TIME_TO_WAIT, 0};
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	
	rc = select(s + 1, &readfds, NULL, NULL, &tv);
	if(rc == 0) {
		fprintf(stderr,"No answer after %d seconds.\n",TIME_TO_WAIT);
        exit(1);
	}
	else if(rc < 0) {
		perror("select");
        exit(1);
	}
    //on attend la fin de modification du fichier
    printf("We receive an answer, file in modification.\n");
    int fd = open(DATA_FILE,O_RDONLY);
    if(fd <= 0) {
        printf("another instance of the code have deleted the file\n");
        exit(1);
    }
   
    FILE* f = fopen(DATA_FILE,"r");
    if(f == NULL) {
        fprintf(stderr,"error %s : %s\n",DATA_FILE,strerror(errno));
        exit(1);
    }
    while(flock(fd,LOCK_EX) != 0) {
        sleep(1);
        perror("what");
    }
   
    char tab[25+2*MAX_DATA_SIZE];
    char hexa[3];
    char tochar[2];
    short found = 0;
    hexa[2] = '\0';
    while(fgets(tab,25+2*MAX_DATA_SIZE,f)) {
        char* data = strstr(tab," ");
        data[0] = '\0';
        data+=1;
       
        if(ID==NULL || strcmp(ID,tab) == 0) {
            found = 1;
            char res[strlen(data)/2 + 1];
            res[strlen(data)/2] = '\0';
            printf("%s: ",tab);
            for(int i = 0;i<strlen(data);i+=2) {
                memcpy(hexa,data+i,2);
                int s = strtol(hexa, NULL, 16);
                snprintf(tochar,2,"%c",s);
                memcpy(res+i/2,tochar,1);
            }
            printf("%s\n",res);
        }
    }
    flock(fd,LOCK_UN);
    fclose(f);
    close(fd);
    remove(DATA_FILE);
	close(s);
    
    if(!found) {
        printf("ID not found\n");
    }
	exit(0);
}