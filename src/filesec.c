#include "global.h"
#include "debug.h"
#include "filesec.h"
#include "getpass.h"
#include "crypt.h"
#include "io.h"

int filesec(const char *src, char *dst, char mode, char *passfile)
{
    enter(USER_CALL, __func__, "%s, %s, %c, %s", src, dst, mode, ((passfile) != NULL ? (passfile) : "NULL"));
    int retval = EXIT_SUCCESS, diff_fs = FALSE;
    char *password, *cwd = NULL;
    unsigned char *hash, *fetched_key;
    int fd_infile, fd_tmpfile, fd_outfile;
    size_t outfile_size;
    struct stat infile_stat, outfile_stat;
    struct statvfs infile_statfs, outfile_statfs;
    char tmp_filename[] = TMP_FILENAME;
    int (*crypt_ptr)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *) = NULL;
    //OPEN FILES
    //open infile
    //check if stdin
    if (!(strncmp("-\0", src, 2)))
    {
        fd_infile = STDIN_FILENO;
        //check for passfile
        if (passfile == NULL)
        {
            fprintf(stderr, KRED "Error: Passfile is not provided. This is required when reading from STDIN\n" KNRM);
            retval = EXIT_FAILURE;
            goto exit;
        }
    }
    else
    {
        enter(SYS_CALL, "open", "%s, %s", src, "O_RDONLY");
        if ((fd_infile = open(src, O_RDONLY)) < 0)
        {
            perror(KRED "Failed to open file" KNRM);
            leave(SYS_CALL, "open", "%s", strerror(errno));
            retval = EXIT_FAILURE;
            goto exit;
        }
        leave(SYS_CALL, "open", "%d", fd_infile);
    }
    //open outfile
    //check if stdout
    if (!(strncmp("-\0", dst, 2)))
    {
        fd_outfile = STDOUT_FILENO;
    }
    else
    {
        enter(SYS_CALL, "open", "%s, %s", dst, "O_WRONLY | O_CREAT | O_TRUNC");
        if ((fd_outfile = open(dst, O_WRONLY | O_CREAT | O_TRUNC, infile_stat.st_mode)) < 0)
        {
            perror(KRED "Failed to open file" KNRM);
            leave(SYS_CALL, "open", "%s", strerror(errno));
            fprintf(stderr, KYLW "Outfile is located at %s\n" KYLW, tmp_filename);
            goto close_fdin;
        }
        leave(SYS_CALL, "open", "%d", fd_outfile);
    }
    //open tmpfile
    enter(LIB_CALL, "mkstemp", "%s", tmp_filename);
    if ((fd_tmpfile = mkstemp(tmp_filename)) < 0)
    {
        perror(KRED "Failed to make tmp file" KNRM);
        leave(LIB_CALL, "mkstemp", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto close_fdout;
    }
    leave(LIB_CALL, "mkstemp", "%d", fd_tmpfile);
    //STAT
    enter(SYS_CALL, "fstat", "%d, %p", fd_infile, &infile_stat);
    if (fstat(fd_infile, &infile_stat) < 0)
    {
        perror(KRED "Failed to get file status" KNRM);
        leave(SYS_CALL, "fstat", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
        goto close_fdtmp;
    }
    leave(SYS_CALL, "fstat", "%d", EXIT_SUCCESS);
    enter(SYS_CALL, "fstat", "%d, %p", fd_outfile, &outfile_stat);
    if (fstat(fd_outfile, &outfile_stat) < 0)
    {
        perror(KRED "Failed to get file status" KNRM);
        leave(SYS_CALL, "fstat", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
        goto close_fdtmp;
    }
    leave(SYS_CALL, "fstat", "%d", EXIT_SUCCESS);
    //STATFS
    enter(SYS_CALL, "fstatvfs", "%d, %p", fd_infile, &infile_statfs);
    if (fstatvfs(fd_infile, &infile_statfs) < 0)
    {
        perror(KRED "Failed to get file system status" KNRM);
        leave(SYS_CALL, "fstatvfs", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
        goto close_fdtmp;
    }
    leave(SYS_CALL, "fstatvfs", "%d", EXIT_SUCCESS);
    enter(SYS_CALL, "fstatvfs", "%d, %p", fd_outfile, &outfile_statfs);
    if (fstatvfs(fd_outfile, &outfile_statfs) < 0)
    {
        perror(KRED "Failed to get file system status" KNRM);
        leave(SYS_CALL, "fstatvfs", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
        goto close_fdtmp;
    }
    leave(SYS_CALL, "fstatvfs", "%d", EXIT_SUCCESS);
    //Stat Calc
    if ((infile_stat.st_ino == outfile_stat.st_ino && !(fd_infile == STDIN_FILENO && fd_outfile == STDOUT_FILENO)) && infile_statfs.f_fsid == outfile_statfs.f_fsid)
    {
        fprintf(stderr, KRED "Error: infile and outfile are the same!\n" KNRM);
        retval = EXIT_FAILURE;
        goto close_fdtmp;
    }
    diff_fs = infile_statfs.f_fsid == outfile_statfs.f_fsid ? FALSE : TRUE;
    //GET PASSWORD/HASH
    if (passfile == NULL)
    {
        if (getpass_stdin(&password) != EXIT_SUCCESS)
        {
            fprintf(stderr, KRED "Failed to get password from stdin\n" KNRM);
            retval = EXIT_FAILURE;
            goto close_fdtmp;
        }
    }
    else
    {
        if (getpass_file(&password, passfile) != EXIT_SUCCESS)
        {
            fprintf(stderr, KRED "Failed to get password from file\n" KNRM);
            retval = EXIT_FAILURE;
            goto close_fdtmp;
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
    fetched_key = hash;
    outfile_size = infile_stat.st_size;
    //SELECT MODE + READ/WRITE HASH
    if (mode == ENCRYPT)
    {
        crypt_ptr = encrypt;
        outfile_size += SHA256_DIGEST_SIZE;
        enter(LIB_CALL, "posix_fallocate", "%d, %d, %ld", fd_tmpfile, 0, outfile_size);
        if ((errno = posix_fallocate(fd_tmpfile, 0, outfile_size)) != EXIT_SUCCESS)
        {
            perror(KRED "Failed to get allocate disk space for the file" KNRM);
            leave(LIB_CALL, "posix_fallocate", "%s", strerror(errno));
            retval = EXIT_FAILURE;
            goto free_hash;
        }
        leave(LIB_CALL, "posix_fallocate", "%d", EXIT_SUCCESS);
        if (full_write(fd_tmpfile, hash, SHA256_DIGEST_SIZE) < 0)
        {
            fprintf(stderr, KRED "Failed to write hash to output file\n" KNRM);
            retval = EXIT_FAILURE;
            goto free_hash;
        }
    }
    else if (mode == DECRYPT)
    {
        crypt_ptr = decrypt;
        outfile_size -= SHA256_DIGEST_SIZE;
        enter(LIB_CALL, "posix_fallocate", "%d, %d, %ld", fd_tmpfile, 0, outfile_size);
        if ((errno = posix_fallocate(fd_tmpfile, 0, outfile_size)) != EXIT_SUCCESS)
        {
            perror(KRED "Failed to get allocate disk space for the file" KNRM);
            leave(LIB_CALL, "posix_fallocate", "%s", strerror(errno));
            retval = EXIT_FAILURE;
            goto free_hash;
        }
        leave(LIB_CALL, "posix_fallocate", "%d", EXIT_SUCCESS);
        if (full_read(fd_infile, fetched_key, SHA256_DIGEST_SIZE) < 0)
        {
            fprintf(stderr, KRED "Failed to fetch hash from input file\n" KNRM);
            retval = EXIT_FAILURE;
            goto free_hash;
        }
        //INSECURE BY DESIGN >:(
        //see https://security.stackexchange.com/questions/160808/why-should-memcmp-not-be-used-to-compare-security-critical-data/160855
        if (memcmp(hash, fetched_key, SHA256_DIGEST_SIZE))
        {
            fprintf(stderr, KRED "The password's hash and the prepended hash of the infile do not match!\n" KNRM);
            retval = EXIT_FAILURE;
            goto free_hash;
            //hashes do not match
        }
    }
    else
    {
        //ERROR???????
        retval = EXIT_FAILURE;
        goto free_hash;
    }
    if (crypter(fd_infile, fd_tmpfile, outfile_size, hash, crypt_ptr) != EXIT_SUCCESS)
    {
        fprintf(stderr, KRED "Failed to complete cryptographic function\n" KNRM);
        retval = EXIT_FAILURE;
        goto free_hash;
    }
    //check if differnet FS
    //get tmp stat
    if (fd_outfile == STDOUT_FILENO)
    {
        close(fd_tmpfile);
        fd_tmpfile = open(tmp_filename, O_RDWR);
        if (full_read_write(fd_outfile, fd_tmpfile, outfile_size) < 0)
        {
            fprintf(stderr, KRED "Failed to read+write file\n" KNRM);
            retval = EXIT_FAILURE;
        }
        //unlink
        enter(SYS_CALL, "unlink", "%s", tmp_filename);
        if (unlink(tmp_filename) < 0)
        {
            perror(KRED "Failed to unlink file" KNRM);
            leave(SYS_CALL, "unlink", "%d", -EXIT_FAILURE);
            retval = EXIT_FAILURE;
        }
        else
        {
            leave(SYS_CALL, "unlink", "%d", EXIT_SUCCESS);
        }
    }
    else if (!diff_fs)
    {
        //sendfile
        if (full_sendfile(fd_outfile, fd_tmpfile, 0, outfile_size) < 0)
        {
            fprintf(stderr, KRED "Failed to send file across file systems\n" KNRM);
            fprintf(stderr, KYLW "Outfile is located at %s\n" KYLW, tmp_filename);
            retval = EXIT_FAILURE;
        }
    }
    else
    {
        enter(LIB_CALL, "realpath", "%s, %s", dst, cwd);
        if ((cwd = realpath(dst, cwd)) == NULL)
        {
            perror(KRED "Failed to get the absolute path to the destination" KNRM);
            leave(LIB_CALL, "realpath", "%s", "NULL");
            fprintf(stderr, KYLW "Outfile is located at %s\n" KYLW, tmp_filename);
            retval = EXIT_FAILURE;
            goto free_hash;
        }
        leave(LIB_CALL, "realpath", "%s", cwd);
        //rename
        enter(LIB_CALL, "rename", "%s, %s", tmp_filename, cwd);
        if (rename(tmp_filename, cwd) < 0)
        {
            perror(KRED "Failed to rename the tmp file to destination" KNRM);
            leave(LIB_CALL, "rename", "%d", -EXIT_FAILURE);
            fprintf(stderr, KYLW "Outfile is located at %s\n" KYLW, tmp_filename);
            retval = EXIT_FAILURE;
        }
        else
        {
            leave(LIB_CALL, "rename", "%d", EXIT_SUCCESS);
        }
        if (cwd)
        {
            enter(LIB_CALL, "free", "%p", cwd);
            free(cwd);
            leave(LIB_CALL, "free", "%s", "void");
        }
    }
free_hash:
    if (hash)
    {
        enter(LIB_CALL, "OPENSSL_clear_free", "%p, %ud", hash, SHA256_DIGEST_SIZE);
        OPENSSL_clear_free(hash, (unsigned int)SHA256_DIGEST_SIZE);
        leave(LIB_CALL, "OPENSSL_clear_free", "%s", "void");
    }
close_fdout:
    enter(SYS_CALL, "close", "%d", fd_outfile);
    if (close(fd_outfile) < 0)
    {
        perror(KRED "Failed to close file" KNRM);
        leave(SYS_CALL, "close", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
    }
    else
    {
        leave(SYS_CALL, "close", "%d", EXIT_SUCCESS);
    }
close_fdtmp:
    enter(SYS_CALL, "close", "%d", fd_tmpfile);
    if (close(fd_tmpfile) < 0)
    {
        perror(KRED "Failed to close file" KNRM);
        leave(SYS_CALL, "close", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
    }
    else
    {
        leave(SYS_CALL, "close", "%d", EXIT_SUCCESS);
    }
close_fdin:
    enter(SYS_CALL, "close", "%d", fd_infile);
    if (close(fd_infile) < 0)
    {
        perror(KRED "Failed to close file" KNRM);
        leave(SYS_CALL, "close", "%d", -EXIT_FAILURE);
        retval = EXIT_FAILURE;
    }
    else
    {
        leave(SYS_CALL, "close", "%d", EXIT_SUCCESS);
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
