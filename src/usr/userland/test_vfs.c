#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "libc/lib.h"

#define PRINT_BUFFER_LEN 256
char print_buffer[PRINT_BUFFER_LEN] = {};

#define printf(fmt, ...) \
    sprint(print_buffer, fmt , ##__VA_ARGS__); cwrites(print_buffer)

void test_fd_assignment(void)
{
    fd_t fd0 = fopen("/", O_READ, 0);
    printf("fd0: %d\n", fd0);

    fd_t fd1 = fopen("/etc", O_READ, 0);
    printf("fd1: %d\n", fd1);

    fclose(fd0);

    fd_t fd2 = fopen("/etc/passwd", O_READ, 0);
    printf("fd2: %d\n", fd2);

    fd_t fd3 = fopen("/usr/bin/chattr", O_READ, 0);
    printf("fd3: %d\n", fd3);

    fclose(fd0);
    fclose(fd2);
    fclose(fd3);

    fd_t fd4 = fopen("/usr", O_READ, 0);
    printf("fd4: %d\n", fd4);

    fclose(fd4);
}

void test_rw_locks(void)
{
    fd_t fd0 = fopen("/", O_READ, 0);

    fd_t fd1 = fopen("/", O_READ, 0);
    fd_t fd2 = fopen("/", O_WRITE, 0);
    fd_t fd3 = fopen("/", O_RDWR, 0);
    // Expected 0, 1, error (-10 atow), error (-10 atow)
    printf("fd0: %d, fd1: %d, fd2: %d, fd3: %d\n", fd0, fd1, fd2, fd3);

    fclose(fd0);
    fclose(fd1);

    fd_t fd4 = fopen("/", O_WRITE, 0);

    // Expected 0, error (-10 atow), error (-10 atow), error (-10 atow)
    fd_t fd5 = fopen("/", O_READ, 0);
    fd_t fd6 = fopen("/", O_WRITE, 0);
    fd_t fd7 = fopen("/", O_RDWR, 0);
    printf("fd4: %d, fd5: %d, fd6: %d, fd7: %d\n", fd4, fd5, fd6, fd7);

    fclose(fd4);

    // Expected 0, error (-10 atow), error (-10 atow)
    fd_t fd8  = fopen("/", O_RDWR, 0);
    fd_t fd9  = fopen("/", O_READ, 0);
    fd_t fd10 = fopen("/", O_WRITE, 0);
    printf("fd8: %d, fd9: %d, fd10: %d\n", fd8, fd9, fd10);

    fclose(fd8);
}

USERMAIN(test_vfs)
{
    // cwrites("Hello world!\n");

    // test_fd_assignment();
    test_rw_locks();

    return 0;
}