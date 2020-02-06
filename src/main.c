#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "global.h"
#include "debug.h"
#include "version.h"

#define ENCRYPT 'e'
#define DECRYPT 'd'
#define VERSION 'v'
#define HELP 'h'
#define DBUG 'D'
#define PASSFILE 'p'

static const char *options = "devhD:p:";
static int e_flag, d_flag, p_flag = 0;

int main(int argc, char *argv[])
{
    int retval = EXIT_SUCCESS;
    int opt;
    // char *p_value;
    while ((opt = getopt(argc, argv, options)) != -1)
    {
        switch (opt)
        {
        case ENCRYPT:
            e_flag++;
            break;
        case DECRYPT:
            d_flag++;
            break;
        case VERSION:
            retval = Version(argv[0]);
            goto exit;
            break;
        case HELP:
            goto fail;
            break;
        case DBUG:
            DBGVAL = atoi(optarg);
            break;
        case PASSFILE:
            p_flag++;
            // p_value = optarg;
            break;
        default:
            goto fail;
            break;
        }
    }
    //DEBUG Boiler plate enter
    if ((retval = enter_main(USER_CALL, __func__, argc, argv)) < 0)
        return retval;
    //DEBUG Boiler plate enter
    if (argc <= 1)
        goto fail;
    if (!(e_flag ^ d_flag))
        goto fail;
    goto exit;
fail:
    retval = Usage(argv[0]);
exit:
    //DEBUG Boiler plate leave
    leave(USER_CALL, __func__, retval);
    //DEBUG Boiler plate leave
    return retval;
}

int Usage(char *name)
{
    int retval, result;
    if ((retval = enter(USER_CALL, __func__, 1, name)) < 0)
        return retval;
    result = usage(name);
    if ((retval = leave(USER_CALL, __func__, result)) < 0)
        return retval;
    return EXIT_FAILURE;
}

int usage(char *name)
{
    int retval;
    if ((retval = fprintf(stderr, "Usage: %s [-devh] [-D DBGVAL] [-p PASSFILE] infile outfile\n", name)) < 0)
        return retval;
    else
        return EXIT_SUCCESS;
}