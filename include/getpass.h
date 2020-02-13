#ifndef GETPASS_H
#define GETPASS_H

int getpass_stdin(char **lineptr);
int getpass_file(char **lineptr, char *infile);

#endif