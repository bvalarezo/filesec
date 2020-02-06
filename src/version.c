#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "version.h"
#include "debug.h"
#include "global.h"

int Version(char *name)
{
    int retval, result;
    if ((retval = enter(USER_CALL, __func__, 1, name)) < 0)
        return retval;
    result = version(name);
    if ((retval = leave(USER_CALL, __func__, result)) < 0)
        return retval;
    return EXIT_SUCCESS;
}

int version(char *name)
{
    int retval;
    if ((retval = fprintf(stdout, "%s version %s\n", name, VERSION_NUMBER)) < 0)
        return retval;
    else
        return EXIT_SUCCESS;
}