#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "tlv.h"
#include "package.h"
#include "crypt.h"
#include <signal.h>
#include <time.h>

int s;

void handler() {
	close(s);
	exit(1);
}

int main(int argc, char** argv) {
	int port = DEFAULT_PORT;
	char* IP = "::1";

	if(argc == 2 && strcmp(argv[1],"--help") !=0 ) {
		port = atoi(argv[1]);
	}
	else if(argc != 1) {
		printf("Usage: dazibao_zoo {PORT}\n");
		printf("Send a TLV to change the data of the node every second\n");
		printf("These data are random and each one correspond to an animal\n");
		printf("The diagramms are sent to ::1\n");
		printf("If PORT is not specified, %d is set\n",port);
		exit(1);
	}


	srand(time(NULL));

	// Ouverture Socket
	s = socket(AF_INET6, SOCK_DGRAM, 0);
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

	signal(SIGINT, handler);

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;

	rc = inet_pton(AF_INET6, IP, &sin6.sin6_addr);
	if(rc < 1) {
		perror("inet_pton");
		close(s);
		exit(1);
	}
	sin6.sin6_port = htons(port);

	char* messages[30];
	messages[0] = "voici un potit ronard : π¦";
	messages[1] = "voici un potit poulΓ© : π";
	messages[2] = "voici un potit chieng : π";
	messages[3] = "voici un potit chat : π";
	messages[4] = "voici une potite tortu : π’";
	messages[5] = "voici un potit dragont : π";
	messages[6] = "voici une potite piouvre : π";
	messages[7] = "voici une potite arΓ©gnΓ©e : π·";
	messages[8] = "voici un potit oursse : π»";
	messages[9] = "voici un potit penda : πΌ";
	messages[10] = "voici un potit canar : π¦";
	messages[11] = "voici une potite abΓ©ye : π";
	messages[12] = "voici un potit escargo : π";
	messages[13] = "voici un potit tirex : π¦";
	messages[14] = "voici un potit chamo : πͺ";
	messages[15] = "voici un potit ecureuille : πΏ";
	messages[16] = "voici une potite voche : π?";
	messages[17] = "voici une potite chove-souri : π¦";
	messages[18] = "voici un potit paingouin : π§";
	messages[19] = "voici une potite likorne : π¦";
	messages[20] = "voici une potite crevΓ¨te : π¦";
	messages[21] = "voici un potit poasson : π";
	messages[22] = "voici une potite balΓ¨ne : π³";
	messages[23] = "voici un potit seinge : π¦";
	messages[24] = "voici une potite souri : π";
	messages[25] = "voici un potit hairisson : π¦";
	messages[26] = "voici un potit cochong : π";
	messages[27] = "voici un potit crapo : πΈ";
	messages[28] = "voici une potite fourmmi : π";
	messages[29] = "voici un potit lyon : π¦";

	while (1) {
		// Building message
		time_t rawtime;
		time(&rawtime);
		struct tm* timeinfo = localtime(&rawtime);

		if (timeinfo->tm_sec != 0) {
			continue;
		}

		char* msg = messages[rand() % 30];
		char message[100];
		memset(message, '\0', 100);
		sprintf(message, "[%02d:%02d] %s", timeinfo->tm_hour, timeinfo->tm_min, msg);

		int buff_size;
		void* buffer = encrypt_message(message, &buff_size);   
		if(buffer == NULL) return 0;
    	TLV mess = change_message((unsigned char*)buffer, buff_size);
		free(buffer);
		if (mess == NULL) {
			close(s);
			exit(1);
		}

		Package paquet = new_paquet();
		int adt = addTLV(paquet, mess);
		if (adt < 0) {
			if (adt == -1) printf("addTLV: Plus de place");
			if (adt == -2) printf("addTLV: Erreur de realloc()");
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
			printf("TLV Type 42 envoyΓ© :\n%s\n\n", message);
			sleep(55);
		}
		free(ready_to_send);
	}
}