#include "global.h"
#include "debug.h"
#include "getpass.h"
#include "io.h"

/*https://www.gnu.org/software/libc/manual/html_node/getpass.html*/

int getpass_stdin(char **lineptr)
{
    enter(USER_CALL, __func__, "%p", lineptr);
    int retval = EXIT_SUCCESS;
    ssize_t nread;
    size_t buff_size;
    struct termios old, new;
    /* Get the page size with sysconf*/
    enter(LIB_CALL, "sysconf", "%s", "_SC_PAGESIZE");
    if ((buff_size = sysconf(_SC_PAGESIZE)) < 0)
    {
        perror(KRED "Failed to get system's page size\n" KNRM);
        leave(LIB_CALL, "sysconf", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "sysconf", "%ld", buff_size);
    /* Back up terminal */
    enter(LIB_CALL, "tcgetattr", "%d, %p", fileno(stdin), &old);
    if (tcgetattr(fileno(stdin), &old) != 0)
    {
        perror(KRED "Failed to backup terminal settings\n" KNRM);
        leave(LIB_CALL, "tcgetattr", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "tcgetattr", "%d", EXIT_SUCCESS);
    new = old;
    new.c_lflag &= ~ECHO;
    /* Set terminal, turning ECHO off */
    enter(LIB_CALL, "tcsetattr", "%d, %s, %p", fileno(stdin), "TCSAFLUSH", &new);
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0)
    {
        perror(KRED "Failed to set terminal settings\n" KNRM);
        leave(LIB_CALL, "tcsetattr", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "tcsetattr", "%d", EXIT_SUCCESS);
    /* Read the passphrase */
    //FPRINTF DOESNT WORK BECAUSE 02, use full write
    if (full_write(fileno(stdout), "Enter Password:", 15) < 0)
    {
        perror(KRED "Failed to write message\n" KNRM);
        retval = EXIT_FAILURE;
        goto restore;
    }
    if ((nread = mygetline(lineptr, buff_size, fileno(stdin))) < 0)
    {
        fprintf(stderr, KRED "\nFailed to get line from terminal\n" KNRM);
        retval = EXIT_FAILURE;
        goto restore;
    }
restore:
    /* Restore terminal. */
    enter(LIB_CALL, "tcsetattr", "%d, %s, %p ", fileno(stdin), "TCSAFLUSH", &old);
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &old) != 0)
    {
        perror(KRED "Failed to restore terminal settings\n" KNRM);
        leave(LIB_CALL, "tcsetattr", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto free_exit;
    }
    leave(LIB_CALL, "tcsetattr", "%d", EXIT_SUCCESS);
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
free_exit:
    if (*lineptr)
    {
        enter(LIB_CALL, "free", "%p", *lineptr);
        free(*lineptr);
        leave(LIB_CALL, "free", "%s", "void");
    }
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int getpass_file(char **lineptr, char *infile)
{
    enter(USER_CALL, __func__, "%p, %s", lineptr, infile);
    int retval = EXIT_SUCCESS;
    ssize_t nread;
    int fd;
    struct stat file_stat;
    size_t buff_size;
    /* Get the page size with sysconf*/
    enter(LIB_CALL, "sysconf", "%s", "_SC_PAGESIZE");
    if ((buff_size = sysconf(_SC_PAGESIZE)) < 0)
    {
        perror(KRED "Failed to get system's page size\n" KNRM);
        leave(LIB_CALL, "sysconf", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "sysconf", "%ld", buff_size);
    /* Set the buff size to either the file size or PAGESIZE*/
    enter(SYS_CALL, "stat", "%s, %p", infile, &file_stat);
    if (stat(infile, &file_stat) < 0)
    {
        perror(KRED "Failed to get file status\n" KNRM);
        leave(SYS_CALL, "stat", "%s", strerror(errno));
        /*buff_size is PageSize*/
    }
    else
    {
        leave(SYS_CALL, "stat", "%d", EXIT_SUCCESS);
        /*Decide between PageSize or FileSize*/
        if (file_stat.st_size > buff_size)
        {
            //add a warning if file size is larger than page size
            fprintf(stderr, KYLW "Warning: passfile \'%s\' is too large! Only reading the first %ld bytes\n" KNRM, infile, buff_size);
        }
        else
        {
            buff_size = file_stat.st_size;
        }
    }
    //we have the buff_size
    enter(SYS_CALL, "open", "%s, %s", infile, "O_RDONLY");
    if ((fd = open(infile, O_RDONLY)) < 0)
    {
        perror(KRED "Failed to open file\n" KNRM);
        leave(SYS_CALL, "open", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(SYS_CALL, "open", "%d", fd);
    if ((nread = mygetline(lineptr, buff_size, fd)) < 0)
    {
        perror(KRED "\nFailed to get line from terminal\n" KNRM);
        retval = EXIT_FAILURE;
        goto file_close;
    }
    leave(LIB_CALL, "getline", "%zu", nread);
file_close:
    enter(SYS_CALL, "close", "%d", fd);
    if (close(fd) < 0)
    {
        perror(KRED "Failed to close file\n" KNRM);
        leave(SYS_CALL, "close", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(SYS_CALL, "close", "%d", EXIT_SUCCESS);
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

/*On Error, return -1*/
ssize_t mygetline(char **lineptr, size_t n, int fd)
{
    enter(USER_CALL, __func__, "%p, %ld, %d", lineptr, n, fd);
    size_t retval = EXIT_SUCCESS;
    char *ptr;
    size_t new_n = 0;
    //fd is the file descriptor, already opened, dont close it
    //no memory allocation for it, so do it
    enter(LIB_CALL, "malloc", "%ld", n);
    if ((*lineptr = malloc(n)) == NULL)
    {
        perror(KRED "Failed to malloc bytes\n" KNRM);
        leave(LIB_CALL, "malloc", "%s", strerror(errno));
        retval = -EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "malloc", "%p", *lineptr);
    while (TRUE)
    {
        enter(SYS_CALL, "read", "%d, %p, %ld", fd, *lineptr, n);
        if ((retval = read(fd, *lineptr, n)) < 0)
        {
            perror(KRED "Error when reading file\n" KNRM);
            leave(SYS_CALL, "read", "%s", strerror(errno));
            retval = -EXIT_FAILURE;
            goto free_exit;
        }
        leave(SYS_CALL, "read", "%ld", retval);
        enter(LIB_CALL, "memchr", "%p, %s, %ld", *lineptr, "\\n", n);
        ptr = (char *)memchr(*lineptr, '\n', n);
        leave(LIB_CALL, "memchr", "%p", ptr);
        if (ptr != NULL)
        {
            break;
        }
    }
    printf("\n");
    //realloc space
    if (!(new_n = (ptr - *lineptr)))
    {
        //NULL PASSWORD, it will free
        fprintf(stderr, KRED "Error: password must not be NULL" KNRM);
        retval = -EXIT_FAILURE;
        goto free_exit;
    }
    enter(LIB_CALL, "realloc", "%p, %ld", *lineptr, new_n * sizeof(char));
    if (!(*lineptr = realloc(*lineptr, new_n * sizeof(char))))
    {
        perror(KRED "Failed to realloc bytes\n" KNRM);
        leave(LIB_CALL, "realloc", "%s", strerror(errno));
        retval = -EXIT_FAILURE;
        goto free_exit;
    }
    leave(LIB_CALL, "realloc", "%p", *lineptr);
    (*lineptr)[new_n] = 0; //get rid of \n
    //number read, after allocation
    enter(LIB_CALL, "strlen", "%p", *lineptr);
    retval = strlen(*lineptr);
    leave(LIB_CALL, "strlen", "%ld", retval);
exit:
    leave(USER_CALL, __func__, "%ld", retval);
    return retval;
free_exit:
    if (*lineptr)
    {
        enter(LIB_CALL, "free", "%p", *lineptr);
        free(*lineptr);
        leave(LIB_CALL, "free", "%s", "void");
    }
    leave(USER_CALL, __func__, "%ld", retval);
    return retval;
}
