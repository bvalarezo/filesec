#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

int DBGVAL;

int enter_main(int enter_policy, const char *function_name, int argc, char **argv)
{
    int i;
    if (enter_policy & DBGVAL)
    {
        fprintf(stderr, KYLW "Entering... %s(", function_name);
        if (ARG_CALL & DBGVAL)
        {
            fprintf(stderr, "%d, [", argc);
            for (i = 1; i <= argc; i++)
            {
                fprintf(stderr, "%s", argv[i - 1]);
                if (i != argc)
                    fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "])\n" KNRM);
    }
    return EXIT_SUCCESS;
}
