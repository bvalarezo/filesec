#ifndef FILESEC_H
#define FILESEC_H

#include <limits.h>
#include <unistd.h>

#define TMP_FILENAME "filesec_XXXXXX"

int filesec(const char *src, char *dst, char mode, char *passfile);

#endif