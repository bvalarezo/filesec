#include "global.h"
#include "debug.h"
#include "io.h"

int full_read(int fd, void *buf, size_t count)
{
    enter(USER_CALL, __func__, "%d, %p, %ld", fd, buf, count);
    int retval = 0;
    size_t bytes_to_read = count;
    int bytes_read = 0;
    while (bytes_to_read > 0)
    {
        enter(SYS_CALL, "read", "%d, %p, %ld", fd, buf, bytes_to_read);
        if ((bytes_read = read(fd, buf, bytes_to_read)) < 0)
        {
            perror(KRED "Error when reading file" KNRM);
            leave(SYS_CALL, "read", "%d", bytes_read);
            retval = -EXIT_FAILURE;
            goto exit;
        }
        if (bytes_read == 0) //EOF
        {
            leave(SYS_CALL, "read", "%d", bytes_read);
            break;
        }
        //else, it was a partial read
        leave(SYS_CALL, "read", "%d", bytes_read);
        bytes_to_read -= bytes_read;
        retval += bytes_read;
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int full_write(int fd, const void *buf, size_t count)
{
    enter(USER_CALL, __func__, "%d, %p, %ld", fd, buf, count);
    int retval = EXIT_SUCCESS;
    size_t bytes_to_write = count;
    int bytes_written = 0;
    while (bytes_to_write > 0)
    {
        enter(SYS_CALL, "write", "%d, %p, %ld", fd, buf, bytes_to_write);
        if ((bytes_written = write(fd, buf, bytes_to_write)) < 0)
        {
            perror(KRED "Error when writing file" KNRM);
            leave(SYS_CALL, "write", "%d", bytes_written);
            retval = -EXIT_FAILURE;
            goto exit;
        }
        leave(SYS_CALL, "write", "%d", bytes_written);
        bytes_to_write -= bytes_written;
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int full_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    enter(USER_CALL, __func__, "%d, %d, %p, %ld", out_fd, in_fd, offset, count);
    int retval = EXIT_SUCCESS;
    size_t bytes_to_send = count;
    int bytes_sent = 0;
    while (bytes_to_send > 0)
    {
        enter(SYS_CALL, "sendfile", "%d, %d, %p, %ld", out_fd, in_fd, offset, bytes_to_send);
        if ((bytes_sent = sendfile(out_fd, in_fd, offset, bytes_to_send)) < 0)
        {
            perror(KRED "Error when sending file" KNRM);
            leave(SYS_CALL, "sendfile", "%d", bytes_sent);
            retval = -EXIT_FAILURE;
            goto exit;
        }
        leave(SYS_CALL, "sendfile", "%d", bytes_sent);
        bytes_to_send -= bytes_sent;
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int full_read_write(int out_fd, int in_fd, size_t count)
{
    enter(USER_CALL, __func__, "%d, %d, %ld", out_fd, in_fd, count);
    int retval = EXIT_SUCCESS;
    size_t bytes_to_send = count, page_size;
    int bytes_sent = 0;
    unsigned char *bucket;
    enter(LIB_CALL, "sysconf", "%s", "_SC_PAGESIZE");
    if ((page_size = sysconf(_SC_PAGESIZE)) < 0)
    {
        perror(KRED "Failed to get system's page size\n" KNRM);
        leave(LIB_CALL, "sysconf", "%s", strerror(errno));
        retval = EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "sysconf", "%ld", page_size);
    //malloc
    enter(LIB_CALL, "malloc", "%ld", page_size);
    if ((bucket = malloc(page_size)) == NULL)
    {
        perror(KRED "Failed to malloc bytes" KNRM);
        leave(LIB_CALL, "malloc", "%s", "NULL");
        retval = -EXIT_FAILURE;
        goto exit;
    }
    leave(LIB_CALL, "malloc", "%p", bucket);
    while (bytes_to_send > 0)
    {
        if (bytes_to_send < page_size)
            page_size = bytes_to_send;
        //full read infile to inbuffer
        if ((bytes_sent = full_read(in_fd, bucket, page_size)) < 0)
        {
            perror(KRED "Failed to read\n" KNRM);
            retval = -EXIT_FAILURE;
            goto free;
        }
        //full write outfile from outbucket
        if (full_write(out_fd, bucket, bytes_sent) < 0)
        {
            retval = -EXIT_FAILURE;
            goto free;
        }
        enter(LIB_CALL, "memset", "%p, %d, %d", bucket, 0, bytes_sent);
        memset(bucket, 0, bytes_sent);
        leave(LIB_CALL, "memset", "%p", bucket);
        bytes_to_send -= bytes_sent;
    }
free:
    free(bucket);
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}