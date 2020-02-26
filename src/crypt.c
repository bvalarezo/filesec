#include "global.h"
#include "debug.h"
#include "io.h"
#include "crypt.h"

int crypter(int fd_infile, int fd_outfile, size_t outfile_size, unsigned char *key, int (*crypt_ptr)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *))
{
    enter(USER_CALL, __func__, "%d, %d, %ld, %p, %p", fd_infile, fd_outfile, outfile_size, key, crypt_ptr);
    int retval = EXIT_SUCCESS;
    unsigned char *iv;
    int i, buckets, bucket_len, iv_len;
    long page_size;
    unsigned char *in_bucket, *out_bucket;
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
    if (outfile_size % page_size)
        buckets++;
    enter(LIB_CALL, "malloc", "%d", IV_SIZE);
    if ((iv = malloc(IV_SIZE)) == NULL)
    {
        perror(KRED "Failed to malloc bytes" KNRM);
        leave(LIB_CALL, "malloc", "%s", "NULL");
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "malloc", "%p", iv);
    enter(LIB_CALL, "malloc", "%ld", page_size);
    if ((in_bucket = malloc(page_size)) == NULL)
    {
        perror(KRED "Failed to malloc bytes" KNRM);
        leave(LIB_CALL, "malloc", "%s", "NULL");
        retval = EXIT_FAILURE;
        goto free_iv;
    }
    leave(LIB_CALL, "malloc", "%p", iv);
    enter(LIB_CALL, "malloc", "%ld", page_size);
    if ((out_bucket = malloc(page_size)) == NULL)
    {
        perror(KRED "Failed to malloc bytes" KNRM);
        leave(LIB_CALL, "malloc", "%s", "NULL");
        retval = EXIT_FAILURE;
        goto free_in;
    }
    leave(LIB_CALL, "malloc", "%p", iv);
    //Set the IV
    enter(SYS_CALL, "getrandom", "%p, %d, %d", iv, IV_SIZE, 0);
    if ((iv_len = getrandom(iv, IV_SIZE, 0)) < 0)
    {
        perror(KRED "Failed to fill random bytes to IV" KNRM);
        leave(SYS_CALL, "getrandom", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
        goto free_out;
    }
    leave(SYS_CALL, "getrandom", "%d", iv_len);
    for (i = 0; i < buckets; i++)
    {
        //full read infile to inbuffer
        if ((bucket_len = full_read(fd_infile, in_bucket, page_size)) < 0)
        {
            perror(KRED "Failed to read\n" KNRM);
            retval = EXIT_FAILURE;
            goto free_out;
        }
        //crypt inbucket to outbucket
        if ((bucket_len = crypt_ptr(in_bucket, bucket_len, key, 0, out_bucket)) < 0)
        {
            retval = EXIT_FAILURE;
            goto free_out;
        }
        //full write outfile from outbucket
        if (full_write(fd_outfile, out_bucket, bucket_len) < 0)
        {
            retval = EXIT_FAILURE;
            goto free_out;
        }
        enter(LIB_CALL, "memset", "%p, %d, %ld", out_bucket, 0, page_size);
        memset(out_bucket, 0, page_size);
        leave(LIB_CALL, "memset", "%p", out_bucket);
        enter(LIB_CALL, "memset", "%p, %d, %ld", in_bucket, 0, page_size);
        memset(in_bucket, 0, page_size);
        leave(LIB_CALL, "memset", "%p", in_bucket);
    }
free_out:
    if (out_bucket)
    {
        enter(LIB_CALL, "free", "%p", out_bucket);
        free(out_bucket);
        leave(LIB_CALL, "free", "%s", "void");
    }
free_in:
    if (in_bucket)
    {
        enter(LIB_CALL, "free", "%p", in_bucket);
        free(in_bucket);
        leave(LIB_CALL, "free", "%s", "void");
    }
free_iv:
    if (iv)
    {
        enter(LIB_CALL, "free", "%p", iv);
        free(iv);
        leave(LIB_CALL, "free", "%s", "void");
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

/*https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption*/
/*On error, returns -1, else it returns ciphertext_len*/
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
    enter(USER_CALL, __func__, "%p, %d, %p, %p, %p", plaintext, plaintext_len, key, iv, ciphertext);
    EVP_CIPHER_CTX *ctx;
    int len, ciphertext_len, retval;
    /* Create and initialise the context */
    enter(LIB_CALL, "EVP_CIPHER_CTX_new", "%s", "");
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        fprintf(stderr, KRED "Failed to create cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_CIPHER_CTX_new", "%p", ctx);
        retval = -EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "EVP_CIPHER_CTX_new", "%p", ctx);
    enter(LIB_CALL, "EVP_EncryptInit_ex", "%p, %s, %s, %p, %p", ctx, "EVP_aria_256_ctr()", "NULL", key, iv);
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aria_256_ctr(), NULL, key, iv))
    {
        fprintf(stderr, KRED "Failed to initialize cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_EncryptInit_ex", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_EncryptInit_ex", "%d", 1);
    //ENCRYPT//
    enter(LIB_CALL, "EVP_EncryptUpdate", "%p, %p, %p, %p, %d", ctx, ciphertext, &len, plaintext, plaintext_len);
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        fprintf(stderr, KRED "Failed to update cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_EncryptUpdate", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_EncryptUpdate", "%d", 1);
    ciphertext_len = len;
    enter(LIB_CALL, "EVP_EncryptFinal_ex", "%p, %p, %p", ctx, ciphertext + len, &len);
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    {
        fprintf(stderr, KRED "Failed to finalize cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_EncryptFinal_ex", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_EncryptFinal_ex", "%d", 1);
    retval = ciphertext_len + len;
    //EVP_aria_256_ctr
free:
    if (ctx)
    {
        enter(LIB_CALL, "EVP_CIPHER_CTX_free", "%p", ctx);
        EVP_CIPHER_CTX_free(ctx);
        leave(LIB_CALL, "EVP_CIPHER_CTX_free", "%s", "void");
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
    enter(USER_CALL, __func__, "%p, %d, %p, %p, %p", ciphertext, ciphertext_len, key, iv, plaintext);
    EVP_CIPHER_CTX *ctx;
    int len, plaintext_len, retval;
    /* Create and initialise the context */
    enter(LIB_CALL, "EVP_CIPHER_CTX_new", "%s", "");
    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        fprintf(stderr, KRED "Failed to create cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_CIPHER_CTX_new", "%p", ctx);
        retval = -EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "EVP_CIPHER_CTX_new", "%p", ctx);
    enter(LIB_CALL, "EVP_DecryptInit_ex", "%p, %s, %s, %p, %p", ctx, "EVP_aria_256_ctr()", "NULL", key, iv);
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aria_256_ctr(), NULL, key, iv))
    {
        fprintf(stderr, KRED "Failed to initialize cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_DecryptInit_ex", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_DecryptInit_ex", "%d", 1);
    //DECRYPT//
    enter(LIB_CALL, "EVP_DecryptUpdate", "%p, %p, %p, %p, %d", ctx, plaintext, &len, ciphertext, ciphertext_len);
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    {
        fprintf(stderr, KRED "Failed to update cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_DecryptUpdate", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_DecryptUpdate", "%d", 1);
    plaintext_len = len;
    enter(LIB_CALL, "EVP_DecryptFinal_ex", "%p, %p, %p", ctx, plaintext + len, &len);
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    {
        fprintf(stderr, KRED "Failed to finalize cipher context \n" KNRM);
        leave(LIB_CALL, "EVP_DecryptFinal_ex", "%d", 0);
        retval = -EXIT_FAILURE;
        goto free;
    }
    leave(LIB_CALL, "EVP_DecryptFinal_ex", "%d", 1);
    retval = plaintext_len + len;
free:
    if (ctx)
    {
        enter(LIB_CALL, "EVP_CIPHER_CTX_free", "%p", ctx);
        EVP_CIPHER_CTX_free(ctx);
        leave(LIB_CALL, "EVP_CIPHER_CTX_free", "%s", "void");
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int sha256(const unsigned char *message, size_t message_len, unsigned char **digest)
{
    enter(USER_CALL, __func__, "%s, %ld, %p", message, message_len, digest);
    enter(LIB_CALL, "EVP_MD_size", "%s", "EVP_sha256()");
    unsigned int digest_len = EVP_MD_size(EVP_sha256());
    leave(LIB_CALL, "EVP_MD_size", "%ud", digest_len);
    int retval = EXIT_SUCCESS;
    EVP_MD_CTX *mdctx;
    /* Create and initialise the context */
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
    /* Create the digest(hash) */
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