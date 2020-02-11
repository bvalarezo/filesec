#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "global.h"
#include "debug.h"
#include "version.h"

static const char *options = "devhD:p:";
static int e_flag, d_flag, p_flag, v_flag = 0;

int main(int argc, char *argv[])
{
    const char *infile;
    char *outfile;
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
            v_flag++;
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
    if (v_flag)
    {
        retval = Version(argv[0]);
        goto exit;
    }
    if (!(e_flag ^ d_flag))
    {
        fprintf(stderr, KRED "Please specify either -e or -d\n" KNRM);
        goto fail;
    }
    infile = argv[optind];
    outfile = argv[++optind];
    if (argv[++optind])
    {
        fprintf(stderr, KRED "Too many arguments!\nPlease specify only a singular infile and singular outfile\n" KNRM);
        goto fail;
    }
    if (e_flag)
    {
        // retval = crypter();
        //ENCRYPT
    }
    else if (d_flag)
    {
        //DECRYPT
    }
    goto exit;
fail:
    retval = Usage(argv[0]);
exit:
    //DEBUG Boiler plate leave
    leave(USER_CALL, "%d", retval);
    //DEBUG Boiler plate leave
    return retval;
}

int Usage(char *name)
{
    int result;
    enter(USER_CALL, "%s", name);
    result = usage(name);
    leave(USER_CALL, "%d", result);
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