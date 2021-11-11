#include "crypt.h"

int generate_keys() {
	BIGNUM* bne = NULL;
    RSA *keypair;
    BIO *pri;
    BIO *pub;

    int	ret = 0;
	int	bits = 1024;
	unsigned long e = RSA_F4;

    // Création de la pair de clés RSA
	bne = BN_new();
	ret = BN_set_word(bne, e);
	if(ret != 1){
        BN_free(bne);
        return -1;
	}
	keypair = RSA_new();
	ret = RSA_generate_key_ex(keypair, bits, bne, NULL);
	if(ret != 1){
		RSA_free(keypair);
	    BN_free(bne);
        return -1;
	}

    // Écriture de la clé public
	pub = BIO_new_file("public.pem", "w+");
	ret = PEM_write_bio_RSAPublicKey(pub, keypair);
	if(ret != 1) {
        BIO_free(pub);
        RSA_free(keypair);
        BN_free(bne);
        return -1;
	}
    mkdir(".keys", 0700);
    rename("public.pem", ".keys/public.pem");

    // Écriture de la clé privée
	pri = BIO_new_file("private.pem", "w+");
	ret = PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
    if(ret != 1) {
        BIO_free(pub);
        BIO_free(pri);
        RSA_free(keypair);
        BN_free(bne);
        return -1;
	}
    rename("private.pem", ".keys/private.pem");

    // Libération des ressources
    BIO_free(pub);
	BIO_free(pri);
	RSA_free(keypair);
	BN_free(bne);
    return 0;
}


void* encrypt_message(char* message, int* size) {
    int fd = open(".keys/public.pem", O_RDONLY);
    if (fd < 1) {
        perror("public.pem");
        return NULL;
    } else {
        close(fd);
    }

    BIO* pub = BIO_new_file(".keys/public.pem", "rt");
    RSA* keypair = PEM_read_bio_RSAPublicKey(pub, NULL, NULL, NULL);

    int encrypt_len = RSA_size(keypair);
    char* encrypted = malloc(encrypt_len);
    
    int ret = RSA_public_encrypt(strlen(message)+1, (unsigned char*)message, (unsigned char*)encrypted, keypair, RSA_PKCS1_OAEP_PADDING);
    if(ret == -1) {
        printf("decrypting failed.");
        free(encrypted);
        RSA_free(keypair);
        BIO_free(pub);
    }
    RSA_free(keypair);
    BIO_free(pub);

    *size = encrypt_len;
    return encrypted;
}


char* decrypt_buffer(void* buffer, size_t buff_len) {
    int fd = open(".keys/private.pem", O_RDONLY);
    if (fd < 1) {
        perror("private.pem");
        return NULL;
    } else {
        close(fd);
    }

    BIO* pri = BIO_new_file(".keys/private.pem", "rt");
    RSA* keypair = PEM_read_bio_RSAPrivateKey(pri, NULL, NULL, NULL);
    char* decrypted = malloc(buff_len);

    int ret = RSA_private_decrypt(buff_len, (unsigned char*)buffer, (unsigned char*)decrypted, keypair, RSA_PKCS1_OAEP_PADDING);
    if (ret == -1) {
        printf("decrypting failed.");
        free(decrypted);
        RSA_free(keypair);
        BIO_free(pri);
        return NULL;
    }
    RSA_free(keypair);
    BIO_free(pri);
    return decrypted;
}
