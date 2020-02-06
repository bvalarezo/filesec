#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

int DBGVAL;

int enter(int enter_policy, const char *function_name, int num_args, ...)
{
    int retval, i;
    char *arg;
    va_list ap;
    if (enter_policy & DBGVAL)
    {
        if ((retval = fprintf(stderr, "Entering... %s(", function_name)) < 0)
            return retval;
        if (ARG_CALL & DBGVAL)
        {
            va_start(ap, num_args);
            for (i = 1; i <= num_args; i++)
            {
                arg = va_arg(ap, char *);
                if (fprintf(stderr, "%s", arg) < 0)
                    return retval;
                if (i != num_args)
                    if (fprintf(stderr, ", ") < 0)
                        return retval;
            }
            va_end(ap);
        }
        if ((retval = fprintf(stderr, ")\n")) < 0)
            return retval;
    }
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
}

int enter_main(int enter_policy, const char *function_name, int argc, char **argv)
{
    int retval, i;
    if (enter_policy & DBGVAL)
    {
        if ((retval = fprintf(stderr, "Entering... %s(", function_name)) < 0)
            return retval;
        if (ARG_CALL & DBGVAL)
        {
            if ((retval = fprintf(stderr, "%d, ", argc)) < 0)
                return retval;
            for (i = 1; i <= argc; i++)
            {
                if ((retval = fprintf(stderr, "%s", argv[i - 1])) < 0)
                    return retval;
                if (i != argc)
                    if ((retval = fprintf(stderr, ", ")) < 0)
                        return retval;
            }
        }
        if ((retval = fprintf(stderr, ")\n")) < 0)
            return retval;
    }
    return EXIT_SUCCESS;
}
