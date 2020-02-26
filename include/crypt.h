#ifndef CRYPT_H
#define CRYPT_H

#include <unistd.h>
#include <sys/random.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

#define SHA256_DIGEST_SIZE 32
#define IV_SIZE 16

int crypter(int fd_infile, int fd_outfile, size_t outfile_size, unsigned char *key, int (*crypt_ptr)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *));
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);
int sha256(const unsigned char *message, size_t message_len, unsigned char **digest);
#endif