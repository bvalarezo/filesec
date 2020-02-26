#include "version.h"
#include "debug.h"
#include "global.h"

int version(char *name)
{
    enter(USER_CALL, __func__, "%s", name);
    int retval;
    fprintf(stdout, "%s version %.1f\n", name, VERSION_NUMBER);
    retval = EXIT_SUCCESS;
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}