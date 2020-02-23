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
        enter(SYS_CALL, "read", "%d, %p, %ld", fd, buf, bytes_to_read); //add EOF handling
        if ((bytes_read = read(fd, buf, bytes_to_read)) < 0)
        {
            perror(KRED "Error when reading file\n" KNRM);
            leave(SYS_CALL, "read", "%s", strerror(errno));
            retval = -EXIT_FAILURE;
            goto exit;
        }
        if (bytes_read < bytes_to_read)
        {
            //TODO
            //CHECK IF EOF
            //else, it was a partial read
        }
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
            leave(SYS_CALL, "read", "%s", strerror(errno));
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
