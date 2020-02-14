#ifndef IO_H
#define IO_H

#include <unistd.h>

/* Performs a full read up to the count. It will partially read if need be*/
int full_read(int fd, void *buf, size_t count);
/* Performs a full write up to the count. It will partially write if need be*/
int full_write(int fd, const void *buf, size_t count);

#endif