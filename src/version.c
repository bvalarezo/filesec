#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "version.h"
#include "debug.h"
#include "global.h"

int Version(char *name)
{
    int result;
    enter(USER_CALL, "%s", name);
    result = version(name);
    leave(USER_CALL, "%d", result);
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