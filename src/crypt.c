#include "global.h"
#include "debug.h"
#include "crypt.h"

int crypter(int fd_infile, int fd_outfile, size_t outfile_size, unsigned char *key, int (*crypt_ptr)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *))
{
    enter(USER_CALL, __func__, "%d, %d, %ld, %p, %p", fd_infile, fd_outfile, outfile_size, key, crypt_ptr);
    int retval = EXIT_SUCCESS;
    int i, buckets;
    long page_size;
    enter(LIB_CALL, "sysconf", "%s", "_SC_PAGESIZE");
    if ((page_size = sysconf(_SC_PAGESIZE)) < 0)
    {
        perror(KRED "Failed to get system's page size\n" KNRM);
        leave(LIB_CALL, "sysconf", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "sysconf", "%ld", page_size);
    buckets = (int)(outfile_size / page_size);
    if ((outfile_size % page_size))
        buckets++;
    for (i = 0; i < buckets; i++)
    {
        //full read infile to inbuffer
        //crypt
        //full write outfile from cipher
    }
    /*
    file to read is open
    file to write is open
    crypt_ptr tells us what todo(encrypt/decrypt)
    /use bucketing system
    num_of_buckets = file.size // PAGE_SIZE + 1 (?)
    for(i = 0; i < num_of_buckets; i++)
        read a PAGESIZE of the src onto the heap
        write what was read
    */
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
    //EVP_aria_256_ctr
    /*
    we open the file in src
    we choose an ecryption algorithm
    password = key
    read from src
    allocate file system storage
    write to a tmp location on the FS(/tmp/)
    once the read is complete, finish writing, then move the .tmp to the dst
    */
    return 1;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
    return 1;
}

int sha256(const unsigned char *message, size_t message_len, unsigned char **digest)
{
    enter(USER_CALL, __func__, "%s, %ld, %p", message, message_len, digest);
    enter(LIB_CALL, "EVP_MD_size", "%s", "EVP_sha256()");
    unsigned int digest_len = EVP_MD_size(EVP_sha256());
    leave(LIB_CALL, "EVP_MD_size", "%ud", digest_len);
    int retval = EXIT_SUCCESS;
    EVP_MD_CTX *mdctx;
    enter(LIB_CALL, "EVP_MD_CTX_new", "%s", "");
    if (!(mdctx = EVP_MD_CTX_new()))
    {
        fprintf(stderr, KRED "Failed to create digest context \n" KNRM);
        leave(LIB_CALL, "EVP_MD_CTX_new", "%p", mdctx);
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "EVP_MD_CTX_new", "%p", mdctx);
    enter(LIB_CALL, "EVP_DigestInit_ex", "%p, %s, %s", mdctx, "EVP_sha256()", "NULL");
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
    {
        fprintf(stderr, KRED "Failed to initialize digest context \n" KNRM);
        leave(LIB_CALL, "EVP_DigestInit_ex", "%d", 0);
        retval = EXIT_FAILURE;
        goto free2;
    }
    leave(LIB_CALL, "EVP_DigestInit_ex", "%d", 1);
    enter(LIB_CALL, "EVP_DigestUpdate", "%p, %s, %ld", mdctx, message, message_len);
    if (EVP_DigestUpdate(mdctx, message, message_len) != 1)
    {
        fprintf(stderr, KRED "Failed to update digest context \n" KNRM);
        leave(LIB_CALL, "EVP_DigestUpdate", "%d", 0);
        retval = EXIT_FAILURE;
        goto free2;
    }
    leave(LIB_CALL, "EVP_DigestUpdate", "%d", 1);
    enter(LIB_CALL, "OPENSSL_malloc", "%ud", digest_len);
    if (!(*digest = (unsigned char *)OPENSSL_malloc(digest_len)))
    {
        fprintf(stderr, KRED "Failed to allocate digest memory \n" KNRM);
        leave(LIB_CALL, "OPENSSL_malloc", "%s", "NULL");
        retval = EXIT_FAILURE;
        goto free2;
    }
    leave(LIB_CALL, "OPENSSL_malloc", "%p", *digest);
    enter(LIB_CALL, "EVP_DigestFinal_ex", "%p, %p, %p", mdctx, *digest, &digest_len);
    if (EVP_DigestFinal_ex(mdctx, *digest, &digest_len) != 1)
    {
        fprintf(stderr, KRED "Failed to write digest\n" KNRM);
        leave(LIB_CALL, "EVP_DigestFinal_ex", "%d", 0);
        retval = EXIT_FAILURE;
        goto free1;
    }
    leave(LIB_CALL, "EVP_DigestFinal_ex", "%d", 1);

free2:
    if (mdctx)
    {
        enter(LIB_CALL, "EVP_MD_CTX_free", "%p", mdctx);
        EVP_MD_CTX_free(mdctx);
        leave(LIB_CALL, "EVP_MD_CTX_free", "%s", "void");
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
free1:
    if (digest)
    {
        enter(LIB_CALL, "OPENSSL_clear_free", "%p, %ud", digest, digest_len);
        OPENSSL_clear_free(digest, digest_len);
        leave(LIB_CALL, "OPENSSL_clear_free", "%s", "void");
    }
    goto free2;
}
/*https://wiki.openssl.org/index.php/EVP_Message_Digests*/