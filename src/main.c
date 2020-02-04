#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#define ENCRYPT 'e'
#define DECRYPT 'd'
#define VERSION 'v'
#define HELP 'h'
#define DBGVAL 'D'
#define PASSFILE 'p'

#define VERSION_NUMBER 1.0

static char *options = "devhD:p:";

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, options)) != -1)
    {
        switch (opt)
        {
        case ENCRYPT:
            printf("Encrypt\n");
            break;

        default:
            break;
        }
    }
    printf("Hello World\n");
    return 0;
}