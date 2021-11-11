#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


/**
 *      Fonctions pour chiffrer et déchiffer des messages.
 *      La méthode utilisée est RSA, fournie par la bibliothèque OpenSSL.
 */


/**
 *  Lis la clé publique `public.pem` et chiffre le message dans un buffer.
 *  Retourne le pointeur vers le buffer et écrit dans `size` la taille du buffer.
 */

void* encrypt_message(char* message, int* size);


/**
 *  Lis la clé privée `private.pem` et déchiffre les données du buffer.
 *  Retourne le pointeur vers la chaine de charactère produite.
 */

char* decrypt_buffer(void* buffer, size_t buff_len);


/**
 *  Génère des clés publiques et privées.
 *  Elles seront écrient dans les fichiers `public.pem` et `private.pem`.
 *  Attention : écrase les clés existantes !
 *  Retourne 0 si succès et -1 si échec.
 */

int generate_keys();
