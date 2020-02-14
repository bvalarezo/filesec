#include <getopt.h>
#include <string.h>
#include "global.h"
#include "debug.h"
#include "version.h"
#include "crypt.h"

static const char *options = "devhD:p:";
static int e_flag, d_flag, p_flag, v_flag = 0;

int DBGVAL;

int main(int argc, char *argv[])
{
    const char *infile;
    char *outfile, *p_value = NULL;
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
            p_value = optarg;
            break;
        default:
            goto fail;
            break;
        }
    }
    //DEBUG Boiler plate enter
    enter_main(USER_CALL, argc, argv);
    //DEBUG Boiler plate enter
    if (argc <= 1)
        goto fail;
    if (v_flag)
    {
        retval = version(argv[0]);
        goto exit;
    }
    if (!(e_flag ^ d_flag))
    {
        perror(KRED "Please specify either -e or -d\n" KNRM);
        goto fail;
    }
    infile = argv[optind];
    outfile = argv[++optind];
    if (argv[++optind])
    {
        perror(KRED "Too many arguments!\nPlease specify only a singular infile and singular outfile\n" KNRM);
        goto fail;
    }
    if (e_flag)
    {
        retval = crypter(infile, outfile, ENCRYPT, p_value);
        //ENCRYPT
    }
    else if (d_flag)
    {
        //DECRYPT
        retval = crypter(infile, outfile, DECRYPT, p_value);
    }
    goto exit;
fail:
    retval = usage(argv[0]);
exit:
    //DEBUG Boiler plate leave
    leave(USER_CALL, __func__, "%d", retval);
    //DEBUG Boiler plate leave
    return retval;
}

int usage(char *name)
{
    enter(USER_CALL, __func__, "%s", name);
    int retval;
    fprintf(stderr, "Usage: %s [-devh] [-D DBGVAL] [-p PASSFILE] infile outfile\n", name);
    retval = EXIT_FAILURE;
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}