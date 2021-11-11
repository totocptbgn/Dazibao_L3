CC = gcc 
#A ajouter à CFLAGS
#	Options de débugage
#-D TEST_UNEXCEPTED : affiche les erreurs qui ne devrait pas arriver
#-D TEST_MEMORY : affiche les erreurs de la mémoire
#-D TEST_HISTORY : affichage de historique
#-D TEST_SEND : affichage des paquets envoyés
#-D TEST_RECEIVE : affichage des paquets recus
#-D TEST_DEBUG : affichage divers
#-D TEST_VOISINS : affichage des voisins
#-D TEST_DATA : affichage de la base de données
#-D TEST_WARNING : affiche lorsque l'on envoie un warning
#-D TEST_SPECIAL_DELIVERY : affiche quand on envoie un node State sans demandé
#-D TEST_CALCUL_HASH : affiche quand un hash est calculé
#-D TEST_HASH_COMPARE : 
#	affiche lors de la comparaison de deux network hashs lors de la reception d'un TLV Network Hash sont identiques
#-D TEST_PAUSE : 
#	attend que l'utilisateur rentre entrée après chaque envoie/reception d'un tlv
#-D TEST_IGNORED_ADDRESSES: 
#	affiche les adresses qui sont ignorés et l'adresse IPv4 trouvé qui devra être ignoré 
#	(attention: ne prends pas en compte les IPs déjà voisines)

CFLAGS = -g -Wall -std=c11
OBJS = main.o inondation.o node.o history.o package.o tlv.o database.o crypt.o stun_gestion.o
BIN = dazibao_node 
LIBS = `pkg-config --cflags --libs openssl`

all: $(BIN) clean_objs change zoo read

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(LIBS)

clean_objs: # Clean *.o files
	$(RM) $(OBJS)

clean: # Clean binary files
	$(RM) $(BIN) $(OBJS)

zoo: change_zoo.c package.c tlv.c database.c crypt.c
	$(CC) change_zoo.c package.c tlv.c database.c crypt.c -o dazibao_zoo $(LIBS)

change: change_msg.c package.c tlv.c database.c crypt.c
	$(CC) change_msg.c package.c tlv.c database.c crypt.c -o dazibao_msg $(LIBS)

read: change_msg.c package.c tlv.c database.c crypt.c
	$(CC) print_data.c package.c tlv.c database.c -o dazibao_read $(LIBS)