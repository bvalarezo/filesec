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
            perror(KRED "Error when reading file\n" KNRM);
            leave(SYS_CALL, "read", "%s", strerror(errno));
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
            perror(KRED "Error when writing file\n" KNRM);
            leave(SYS_CALL, "write", "%s", strerror(errno));
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
            perror(KRED "Error when sending file\n" KNRM);
            leave(SYS_CALL, "sendfile", "%s", strerror(errno));
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