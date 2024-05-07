#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "libc/lib.h"

#include "usr/testfs_usr.h"

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

void test_fioctl(void)
{
    // Expected (plus 1 print from the testfs ioctl):
    // res1: -9, res2: 0
    // res3: -9, res4: -2
    // res5: -9, res6: -12
    // res7: -9, res8: -12

    fd_t etc_fd = fopen("/etc", O_READ, 0);
    fd_t libgdi_fd = fopen("/usr/lib/libgdi.so", O_READ, 0);

    printf("etc_fd: %d, libgdi_fd: %d\n", etc_fd, libgdi_fd);

    char *message = "butts";
    int32_t res1 = fioctl(etc_fd, TESTFS_SAY_HI, message);
    int32_t res2 = fioctl(libgdi_fd, TESTFS_SAY_HI, message);

    printf("res1: %d, res2: %d\n", res1, res2);

    int32_t res3 = fioctl(etc_fd, TESTFS_SAY_HI, NULL);
    int32_t res4 = fioctl(libgdi_fd, TESTFS_SAY_HI, NULL);

    printf("res3: %d, res4: %d\n", res3, res4);

    int32_t res5 = fioctl(etc_fd, 2, message);
    int32_t res6 = fioctl(libgdi_fd, 2, message);

    printf("res5: %d, res6: %d\n", res5, res6);

    int32_t res7 = fioctl(etc_fd, 2, NULL);
    int32_t res8 = fioctl(libgdi_fd, 2, NULL);

    printf("res7: %d, res8: %d\n", res7, res8);

    fclose(etc_fd);
    fclose(libgdi_fd);
}

USERMAIN(test_vfs)
{
    // cwrites("Hello world!\n");

    // test_fd_assignment();
    // test_rw_locks();
    test_fioctl();

    return 0;
}