#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "libc/lib.h"

#define PRINT_BUFFER_LEN 256

USERMAIN(test_vfs)
{
    // cwrites("Hello world!\n");

    char print_buffer[PRINT_BUFFER_LEN] = {};

    fd_t fd0 = fopen("/", 0);
    // fd_t fd0 = -1;
    fd_t fd1 = fopen("/etc", 0);
    fd_t fd2 = fopen("/etc/passwd", 0);
    fd_t fd3 = fopen("/usr/bin/chattr", 0);

    sprint(
        print_buffer,
        "fd0: %d, fd1: %d, fd2: %d, fd3: %d\n",
        fd0, fd1, fd2, fd3
    );
    cwrites(print_buffer);

    return 0;
}