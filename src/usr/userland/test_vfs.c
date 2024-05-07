#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "libc/lib.h"

#define PRINT_BUFFER_LEN 256

#define printf(fmt, ...) \
    sprint(print_buffer, fmt , ##__VA_ARGS__); cwrites(print_buffer)

USERMAIN(test_vfs)
{
    // cwrites("Hello world!\n");

    char print_buffer[PRINT_BUFFER_LEN] = {};

    fd_t fd0 = fopen("/", 0);
    printf("fd0: %d\n", fd0);

    fd_t fd1 = fopen("/etc", 0);
    printf("fd1: %d\n", fd1);

    fclose(fd0);

    fd_t fd2 = fopen("/etc/passwd", 0);
    printf("fd2: %d\n", fd2);

    fd_t fd3 = fopen("/usr/bin/chattr", 0);
    printf("fd3: %d\n", fd3);

    fclose(fd0);
    fclose(fd2);
    fclose(fd3);

    fd_t fd4 = fopen("/usr", 0);
    printf("fd4: %d\n", fd4);

    fclose(fd4);

    // sprint(
    //     print_buffer,
    //     "fd0: %d, fd1: %d, fd2: %d, fd3: %d\n",
    //     fd0, fd1, fd2, fd3
    // );
    // cwrites(print_buffer);

    return 0;
}