#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

int DBGVAL;

int enter(int enter_policy, const char *function_name, ...)
{
    int retval;
    if (enter_policy & DBGVAL)
    {
        if ((retval = fprintf(stderr, "Entering... %s(", function_name)) < 0)
            return retval;
        if (ARG_CALL & DBGVAL)
        {
            if (fprintf(stderr, "arg") < 0)
                return retval;
        }
        if ((retval = fprintf(stderr, ")\n")) < 0)
            return retval;
    }
    return 0;
}

int leave(int leave_policy, const char *function_name, int value)
{
    int retval;
    if (leave_policy & DBGVAL)
    {
        if ((retval = fprintf(stderr, "Leaving... %s()", function_name)) < 0)
            return retval;
        if (RETURN_CALL & DBGVAL)
        {
            if (fprintf(stderr, " with return value '%d'", value) < 0)
                return retval;
        }
        if ((retval = fprintf(stderr, "\n")) < 0)
            return retval;
    }
    return 0;
}
