#include "global.h"
#include "debug.h"
#include "getpass.h"
#include "crypt.h"
#include "io.h"

int crypter(const char *src, char *dst, char mode, char *passfile)
{
    enter(USER_CALL, __func__, "%s, %s, %c, %s", src, dst, mode, ((passfile) != NULL ? (passfile) : "NULL"));
    int retval = EXIT_SUCCESS;
    char *password;
    if (passfile == NULL)
    {

        if (getpass_stdin(&password) != EXIT_SUCCESS)
        {
            perror(KRED "Failed to get password from terminal\n" KNRM);
            retval = EXIT_FAILURE;
            goto exit;
        }
    }
    else
    {
        if (getpass_file(&password, passfile) != EXIT_SUCCESS)
        {
            perror(KRED "Failed to get password from file\n" KNRM);
            retval = EXIT_FAILURE;
            goto exit;
        }
    }
    printf("%p\n", password);
    printf("%s\n", password);
    //remember to free password from malloc
    if (mode == ENCRYPT)
    {
        //encryption subroutine
    }
    else if (mode == DECRYPT)
    {
        //decryption subroutine
    }
    //read from src
    //write to dst
    free(password);
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}
