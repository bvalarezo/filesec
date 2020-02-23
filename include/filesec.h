#ifndef FILESEC_H
#define FILESEC_H

#define TMP_FILENAME "/tmp/filesec_XXXXXX"

int filesec(const char *src, char *dst, char mode, char *passfile);

#endif