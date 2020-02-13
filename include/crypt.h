#ifndef CRYPT_H
#define CRYPT_H

int crypter(const char *src, char *dst, char mode, char *passfile);

int getpass_stdin(char **lineptr);
int getpass_file(char **lineptr, char *infile);

#endif