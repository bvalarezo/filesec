#include "global.h"
#include "debug.h"
#include "io.h"

int full_read(int fd, void *buf, size_t count)
{
    enter(USER_CALL, __func__, "%d, %p, %ld", fd, buf, count);
    int retval = EXIT_SUCCESS;
    size_t bytes_to_read = count;
    int bytes_read = 0;
    while (bytes_to_read > 0)
    {
        enter(SYS_CALL, "read", "%d, %p, %ld", fd, buf, bytes_to_read);
        if ((bytes_read = read(fd, buf, bytes_to_read)) < 0)
        {
            perror(KRED "Error when reading file\n" KNRM);
            leave(SYS_CALL, "read", "%s", strerror(errno));
            retval = EXIT_FAILURE;
            goto exit;
        }
        leave(SYS_CALL, "read", "%d", bytes_read);
        bytes_to_read -= bytes_read;
    }
exit:
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}

int full_write(int fd, const void *buf, size_t count)
{
    enter(USER_CALL, __func__, "%d, %p, %ld", fd, buf, count);
    int retval = EXIT_SUCCESS;
    //full write
    leave(USER_CALL, __func__, "%d", retval);
    return retval;
}
