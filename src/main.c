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
    int opt;
    // char *db_value, *p_value;
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
            Version(argv[0]);
            break;
        case HELP:
            usage(argv[0]);
            break;
        case DBUG:
            DBGVAL = atoi(optarg);
            break;
        case PASSFILE:
            p_flag++;
            // p_value = optarg;
            break;
        default:
            usage(argv[0]);
            break;
        }
    }
    //NOT XOR
    Version(argv[0]);
    if (argc <= 1)
        usage(argv[0]);
    printf("Argc = %d\nOpt = %d\n", argc, opt);
    exit(EXIT_SUCCESS);
}

void usage(char *name)
{
    fprintf(stderr, "Usage: %s [-devh] [-D DBGVAL] [-p PASSFILE] infile outfile\n", name);
    exit(EXIT_FAILURE);
}
