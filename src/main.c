#include <getopt.h>
#include "global.h"
#include "debug.h"
#include "version.h"
#include "filesec.h"

static const char *options = "devhD:p:";
static int e_flag, d_flag, p_flag, v_flag = 0;

int DBGVAL;

int main(int argc, char *argv[])
{
    const char *infile;
    char *outfile, *p_value = NULL;
    int retval = EXIT_SUCCESS;
    int opt;
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
        fprintf(stderr, KRED "Incorrect argument amount!\nPlease specify only a singular infile and singular outfile\n" KNRM);
        goto fail;
    }
    if (e_flag)
    {
        retval = filesec(infile, outfile, ENCRYPT, p_value);
        //ENCRYPT
    }
    else if (d_flag)
    {
        //DECRYPT
        retval = filesec(infile, outfile, DECRYPT, p_value);
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

int version(char *name)
{
    enter(USER_CALL, __func__, "%s", name);
    int retval;
    fprintf(stdout, "%s version %.1f\n", name, VERSION_NUMBER);
    retval = EXIT_SUCCESS;
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}