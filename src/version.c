#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "version.h"
#include "debug.h"
#include "global.h"

int Version(char *name)
{
    int retval;
    if ((retval = enter(USER_CALL, __func__, name)) < 0)
        return retval;
    retval = version(name);
    if ((retval = leave(USER_CALL, __func__, retval)) < 0)
        return retval;
    return 0;
}

int version(char *name)
{
    return fprintf(stdout, "%s version %s\n", name, VERSION_NUMBER);
}