#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include "tlv.h"
#include "package.h"
#include "crypt.h"
/*
    Envoie un TLV de type 42 pour change le message
*/

int main(int argc, char** argv) {
    
    if (argc != 3 && argc != 4) {
        printf("Usage: dazibao_msg PORT \"New Message\" {address}\n");
		printf("Send a TLV to change the data of the node on the choosen port and address.\n");
		printf("If address is not specified, ::1 is set by default\n");
        exit(1);
    }    

    char* addr = "::1";
    if(argc == 4) addr = argv[3];

	uint16_t port = atoi(argv[1]);

	srand(time(NULL));

	// Ouverture Socket
	int s = socket(AF_INET6, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(1);
	}

    struct sockaddr_in6 server;
	memset(&server, 0, sizeof(server));
	server.sin6_family = AF_INET6;
	server.sin6_port = htons(CHANGE_PORT);
	
	/*
	// Pour tester les warnings, il faut changer le port
	server.sin6_port = htons(50000);
	*/

	int rc = bind(s, (struct sockaddr*)&server, sizeof(server));
	if(rc < 0) {
		perror("bind");
		exit(1);
	}

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;

	rc = inet_pton(AF_INET6, addr, &sin6.sin6_addr);
	if(rc < 1) {
		perror("inet_pton");
		close(s);
		exit(1);
	}
	sin6.sin6_port = htons(port);

	char* message = argv[2];
    int buff_size;
    void* buffer = encrypt_message(message, &buff_size);   
    if(buffer == NULL) return 0;
    TLV mess = change_message((unsigned char*)buffer, buff_size);
    if (mess == NULL) {
			close(s);
			exit(1);
	}
    Package paquet = new_paquet();
	//int adt = 0;
    int adt = addTLV(paquet, mess);
	
    
	if (adt < 0) {
        if (adt == -1) fprintf(stderr,"addTLV: Plus de place");
        if (adt == -2) fprintf(stderr,"addTLV: Erreur de realloc()");
        close(s);
        exit(1);
    }
    

	
	/*
	// Test warning network state request invalid size (TEST_WARNING)
	TLV content = malloc(NETWORK_STATE_REQUEST_SIZE + 50);
	uint8_t type_test = NETWORK_STATE_REQUEST_SIZE;
	memmove(content, &type_test,TYPE_SIZE);
	uint8_t size_test = 50;
	memmove(content+TYPE_SIZE,&size_test,LENGTH_SIZE);
	addTLV(paquet,content);
	*/

	/*
	// Test warning incorrect hash (TEST_WARNING)
	addTLV(paquet,nodeState(333,10,"jsuisfouédmoi","madata",strlen("madata")));
	*/


	/*
	// Test warning neighbour localhost (TEST_WARNING)
	char IP[IP_SIZE];
	inet_pton(AF_INET6,"::1",IP);
	addTLV(paquet,neighbour(IP,20000));
	*/

    int size = getPackageLength(paquet);
    void* ready_to_send = build(paquet);

	/*
	// Test warning TLV outside range (TEST_WARNING)
	uint8_t size_bug = 200;
	memmove(ready_to_send+TYPE_SIZE+PACKAGE_HEAD_SIZE, &size_bug,LENGTH_SIZE);
	*/

	/*
	// Test warning package outside range (TEST_WARNING)
	uint8_t size_bug = 200;
	memmove(ready_to_send+MAGIC_SIZE + VERSION_SIZE, &size_bug,BODY_LENGTH_SIZE);
	*/

	/*
	// test package magic (TEST_DEBUG)
	uint8_t magic = 5;
	memmove(ready_to_send, &magic,MAGIC_SIZE);
	*/

	/*
	// test package version (TEST_DEBUG)
	uint8_t version = 10;
	memmove(ready_to_send+MAGIC_SIZE, &version,VERSION_SIZE);
	*/


	
    int st = sendto(s, ready_to_send, size, 0, (struct sockaddr*)&sin6, sizeof(sin6));
    if (st < 0) {
        perror("sendto");
        close(s);
        exit(1);
    } else {
        printf("TLV Type 42 envoyé :\n%s\n\n", message);
    }
	
	close(s);
	exit(1);
}