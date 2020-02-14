#ifndef GETPASS_H
#define GETPASS_H

#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int getpass_stdin(char **lineptr);
int getpass_file(char **lineptr, char *infile);

#endif