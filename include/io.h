#ifndef IO_H
#define IO_H

#include <unistd.h>
#include <sys/sendfile.h>

/* Performs a full read up to the count. It will partially read if need be*/
int full_read(int fd, void *buf, size_t count);
/* Performs a full write up to the count. It will partially write if need be*/
int full_write(int fd, const void *buf, size_t count);
/* Performs a full send a file up to the count. It will partially send if need be*/
int full_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

#endif