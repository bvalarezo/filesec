#include "global.h"
#include "debug.h"
#include "filesec.h"
#include "getpass.h"
#include "crypt.h"
#include "io.h"

int filesec(const char *src, char *dst, char mode, char *passfile)
{
    enter(USER_CALL, __func__, "%s, %s, %c, %s", src, dst, mode, ((passfile) != NULL ? (passfile) : "NULL"));
    int retval = EXIT_SUCCESS;
    char *password;
    unsigned char *hash;
    int fd_infile, fd_outfile, fd_tmpfile;
    size_t outfile_size;
    struct stat file_stat;
    char tmp_filename[] = TMP_FILENAME;
    int (*crypt_ptr)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *) = NULL;
    if (passfile == NULL)
    {

        if (getpass_stdin(&password) != EXIT_SUCCESS)
        {
            fprintf(stderr, KRED "Failed to get password from stdin\n" KNRM);
            retval = EXIT_FAILURE;
            goto exit;
        }
    }
    else
    {
        if (getpass_file(&password, passfile) != EXIT_SUCCESS)
        {
            fprintf(stderr, KRED "Failed to get password from file\n" KNRM);
            retval = EXIT_FAILURE;
            goto exit;
        }
    }
    if (sha256((unsigned char *)password, strlen(password), &hash) != EXIT_SUCCESS)
    {
        //failed
        fprintf(stderr, KRED "Failed to hash password\n" KNRM);
        retval = EXIT_FAILURE;
        goto free_password;
    }
    if (password)
    {
        enter(LIB_CALL, "free", "%p", password);
        free(password); //we have the hash, don't need the password anymore
        leave(LIB_CALL, "free", "%s", "void");
    }
    // for (int i = 0; i < SHA256_DIGEST_SIZE; i++)
    //     printf("%02x", hash[i]);
    // crypt_ptr = ((mode) == ENCRYPT ? (encrypt) : (decrypt));
    // src
    enter(SYS_CALL, "stat", "%s, %p", src, &file_stat);
    if (stat(src, &file_stat) < 0)
    {
        perror(KRED "Failed to get file status\n" KNRM);
        leave(SYS_CALL, "stat", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto free_hash;
    }
    leave(SYS_CALL, "stat", "%d", EXIT_SUCCESS);
    outfile_size = file_stat.st_size;
    if (mode == ENCRYPT)
    {
        crypt_ptr = encrypt;
        outfile_size += SHA256_DIGEST_SIZE;
        //remember to append hash in the beginngin
        //encryption subroutine
    }
    else if (mode == DECRYPT)
    {
        crypt_ptr = decrypt;
        outfile_size -= SHA256_DIGEST_SIZE;
        //decryption subroutine
    }
    //open infile
    enter(SYS_CALL, "open", "%s, %s", src, "O_RDONLY");
    if ((fd_infile = open(src, O_RDONLY)) < 0)
    {
        perror(KRED "Failed to open file\n" KNRM);
        leave(SYS_CALL, "open", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto free_hash;
    }
    leave(SYS_CALL, "open", "%d", fd_infile);
    //open tmpfile
    enter(LIB_CALL, "mkstemp", "%s", tmp_filename);
    if ((fd_tmpfile = mkstemp(tmp_filename)) < 0)
    {
        perror(KRED "Failed to make tmp file\n" KNRM);
        leave(LIB_CALL, "mkstemp", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto free_hash;
    }
    leave(LIB_CALL, "mkstemp", "%d", fd_tmpfile);
    //posix_fallacote
    //crypter
    crypter(fd_infile, fd_tmpfile, outfile_size, hash, crypt_ptr);
    if (0 == 1)
    {
        fd_outfile = 1;
        printf("%d", fd_outfile);
    }
    //read from src
    //write to dst
free_hash:
    if (hash)
    {
        enter(LIB_CALL, "OPENSSL_clear_free", "%p, %ud", hash, SHA256_DIGEST_SIZE);
        OPENSSL_clear_free(hash, (unsigned int)SHA256_DIGEST_SIZE);
        leave(LIB_CALL, "OPENSSL_clear_free", "%s", "void");
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
free_password:
    if (password)
    {
        enter(LIB_CALL, "free", "%p", password);
        free(password); //we have the hash, don't need the password anymore
        leave(LIB_CALL, "free", "%s", "void");
    }
    goto exit;
}
