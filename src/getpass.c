
#include <sys/types.h>
#include "global.h"
#include "debug.h"
#include "getpass.h"
#include "io.h"

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
    printf("Enter password: ");
    enter(LIB_CALL, "getline", "%p, %lu, %s", lineptr, buff_size, "stdin");
    if ((nread = getline(lineptr, &buff_size, stdin)) < 0) //WARNING MIGHT BE ILLEGAL
    {
        perror(KRED "\nFailed to get line from terminal\n" KNRM);
        leave(LIB_CALL, "getline", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    (*lineptr)[strcspn(*lineptr, "\n")] = 0; //get rid of \n
    printf("\n");
    leave(LIB_CALL, "getline", "%zu", nread);
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
    /* Realloc lineptr to save memory*/
    enter(LIB_CALL, "realloc", "%p, %ld", *lineptr, (strlen(*lineptr) + 1) * sizeof(char));
    if ((*lineptr = realloc(*lineptr, (strlen(*lineptr) + 1) * sizeof(char))) == NULL)
    {
        perror(KRED "Failed to realloc bytes\n" KNRM);
        leave(LIB_CALL, "realloc", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto free_exit;
    }
    leave(LIB_CALL, "realloc", "%p", *lineptr);
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
free_exit:
    free(*lineptr); //free the buffer
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int getpass_file(char **lineptr, char *infile)
{
    enter(USER_CALL, __func__, "%p, %s", lineptr, infile);
    int retval = EXIT_SUCCESS;
    int fd;
    struct stat file_stat;
    size_t buff_size;
    char *buffer;
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
    enter(SYS_CALL, "open", "%s, %s", infile, "O_RDONLY");
    if ((fd = open(infile, O_RDONLY)) < 0)
    {
        perror(KRED "Failed to open file\n" KNRM);
        leave(SYS_CALL, "open", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(SYS_CALL, "open", "%d", fd);
    enter(SYS_CALL, "fstat", "%d, %p", fd, &file_stat);
    if (fstat(fd, &file_stat) < 0)
    {
        perror(KRED "Failed to get file status\n" KNRM);
        leave(SYS_CALL, "fstat", "%s", strerror(errno));
        /*buff_size is PageSize*/
    }
    else
    {
        leave(SYS_CALL, "fstat", "%d", EXIT_SUCCESS);
        /*Decide between PageSize or FileSize*/
        buff_size = (file_stat.st_size < buff_size ? (file_stat.st_size) : buff_size);
    }
    //we have the buff_size
    enter(LIB_CALL, "malloc", "%ld", buff_size);
    if ((buffer = malloc(buff_size)) == NULL)
    {
        perror(KRED "Failed to malloc bytes\n" KNRM);
        leave(LIB_CALL, "malloc", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto file_close;
    }
    leave(LIB_CALL, "malloc", "%p", buffer);
    if (full_read(fd, buffer, buff_size) < 0)
    {
        perror(KRED "Failed to get fully read bytes from file\n" KNRM);
        retval = EXIT_FAILURE;
        goto file_close;
    }
    char *t;
    t = memchr(buffer, '\n', buff_size);
    printf("%s\n", t);
    //read lineptr until you see \n or EOF
    //remove \n
    //*lineptr should have password
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
