#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "libc/lib.h"

#include "usr/testfs_usr.h"

#define PRINT_BUFFER_LEN 256
char print_buffer[PRINT_BUFFER_LEN] = {};

#define printf(fmt, ...) \
    sprint(print_buffer, fmt , ##__VA_ARGS__); cwrites(print_buffer)

/**
 *
 * root (/)/
 * ├─ etc/
 * │  ├─ passwd
 * │  ├─ group
 * ├─ usr/
 * │  ├─ lib/
 * │  │  ├─ libgdi.so
 * │  ├─ bin/
 * │  │  ├─ chattr
 *
*/

void test_fd_assignment(void)
{
    // For best results, enable debug print in namey.c:resolve_path(...)
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

    fd_t fd5 = fopen("/foo/bar", O_READ, 0);
    printf("fd5: %d\n", fd5);

    fd_t fd6 = fopen("/foo", O_READ, 0);
    printf("fd6: %d\n", fd6);

    fd_t fd7 = fopen("/etc/bar", O_READ, 0);
    printf("fd7: %d\n", fd7);

    fd_t fd8 = fopen("/etc/bar", O_READ, 0);
    printf("fd8: %d\n", fd8);
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


#define TEST_READ_PRINT(fd_name)                                         \
    printf(                                                              \
        #fd_name ": %d, num_read: %d, status %d, read_buffer: \"%s\"\n", \
        fd_name, num_read, status, read_buffer                           \
    )

#define READ_BUFFER_LEN 256

void test_read(void)
{
    // Expected:
    // passwd_fd: 0, num_read: 0, status -12, read_buffer: ""
    // Closing testfs file (in driver) passwd
    // passwd_fd: 0, num_read: 6, status -13, read_buffer: "passwd"
    // passwd_fd: 0, num_read: 0, status -13, read_buffer: ""
    // Closing testfs file (in driver) passwd
    // passwd_fd: 0, num_read: 3, status 0, read_buffer: "pas"
    // passwd_fd: 0, num_read: 3, status -13, read_buffer: "swd"
    // Closing testfs file (in driver) passwd
    // passwd_fd: 0, num_read: 3, status 0, read_buffer: "pas"
    // passwd_fd1: 1, num_read: 6, status -13, read_buffer: "passwd"
    // Closing testfs file (in driver) passwd
    // Closing testfs file (in driver) passwd
    // etc_fd: 0, num_read: 0, status -9, read_buffer: ""

    char read_buffer[READ_BUFFER_LEN] = {};

    // Test read on a write-opened open file
    fd_t passwd_fd = fopen("/etc/passwd", O_WRITE, 0);
    int32_t status = 0;
    uint32_t num_read = fread(passwd_fd, read_buffer, 10, 0, &status);
    TEST_READ_PRINT(passwd_fd);
    __memclr(read_buffer, READ_BUFFER_LEN);

    fclose(passwd_fd);

    // Test read on a read-opened file (where num_to_read == file len)
    passwd_fd = fopen("/etc/passwd", O_READ, 0);
    num_read = fread(passwd_fd, read_buffer, 6, 0, &status);
    TEST_READ_PRINT(passwd_fd);
    __memclr(read_buffer, READ_BUFFER_LEN);

    // Test read on a read-opened file (where rw head == file len)
    num_read = fread(passwd_fd, read_buffer, 6, 0, &status);
    TEST_READ_PRINT(passwd_fd);

    fclose(passwd_fd); // Required because fseek isn't implemented yet

    // Partial read test
    passwd_fd = fopen("/etc/passwd", O_READ, 0);
    num_read = fread(passwd_fd, read_buffer, 3, 0, &status);
    TEST_READ_PRINT(passwd_fd);

    __memclr(read_buffer, READ_BUFFER_LEN);

    num_read = fread(passwd_fd, read_buffer, 6, 0, &status);
    TEST_READ_PRINT(passwd_fd);
    fclose(passwd_fd);

    // Test multiple files on same inode with different rw heads
    passwd_fd = fopen("/etc/passwd", O_READ, 0);
    fd_t passwd_fd1 = fopen("/etc/passwd", O_READ, 0);
    num_read = fread(passwd_fd, read_buffer, 3, 0, &status);
    TEST_READ_PRINT(passwd_fd);

    __memclr(read_buffer, READ_BUFFER_LEN);

    num_read = fread(passwd_fd1, read_buffer, 6, 0, &status);
    TEST_READ_PRINT(passwd_fd1);
    fclose(passwd_fd);
    fclose(passwd_fd1);

    __memclr(read_buffer, READ_BUFFER_LEN);

    // Test read on file that doesn't support it (driver didn't register a read callback in its kfile_ops_t)
    fd_t etc_fd = fopen("/etc", O_READ, 0);
    num_read = fread(etc_fd, read_buffer, 3, 0, &status);
    TEST_READ_PRINT(etc_fd);

    fclose(etc_fd);
}

#define TEST_FSEEK_PRINT(fd_name) \
    printf(#fd_name ": %d, cursor: %d, status: %d\n", fd_name, cursor, status);

void test_fseek(void)
{
    fd_t passwd_fd = fopen("/etc/passwd", O_READ, 0);
    int32_t status = -74; // Bogus value to ensure fseek is setting it properly
    uint32_t cursor = 0;

    // Beginning
    cursor = fseek(passwd_fd, 0, SEEK_SET, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // End
    cursor = fseek(passwd_fd, 0, SEEK_END, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Curr
    cursor = fseek(passwd_fd, 0, SEEK_CURR, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Negative from curr
    cursor = fseek(passwd_fd, -3, SEEK_CURR, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Positive from curr
    cursor = fseek(passwd_fd, 2, SEEK_CURR, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Off beginning of file
    fseek(passwd_fd, 0, SEEK_SET, NULL);
    cursor = fseek(passwd_fd, -1, SEEK_CURR, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Off end of file
    fseek(passwd_fd, 0, SEEK_END, NULL);
    cursor = fseek(passwd_fd, 1, SEEK_CURR, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Negative from end of file
    cursor = fseek(passwd_fd, -2, SEEK_END, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Positive from start of file
    cursor = fseek(passwd_fd, 3, SEEK_SET, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Negative off beginning of file from end of file
    cursor = fseek(passwd_fd, -8, SEEK_SET, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    // Positive off end of file from beginning of file
    cursor = fseek(passwd_fd, 8, SEEK_SET, &status);
    TEST_FSEEK_PRINT(passwd_fd);

    fclose(passwd_fd);
}

void test_write(void)
{
    char read_buffer[READ_BUFFER_LEN] = {};
    char *message = "sudo:x:27:adin";

    fd_t group_fd = fopen("/etc/group", O_RDWR, 0);
    int32_t status = 0;
    uint32_t pos_before = fseek(group_fd, 0, SEEK_CURR, NULL);
    uint32_t num_written = fwrite(group_fd, message, __strlen(message), 0, &status);
    uint32_t pos_after = fseek(group_fd, 0, SEEK_CURR, NULL);

    fseek(group_fd, 0, SEEK_SET, NULL);
    fread(group_fd, read_buffer, pos_after, 0, NULL);

    printf(
        "group_fd: %d, pos_before: %d, status: %d, num_written: %d, pos_after: %d, read_buffer: \"%s\"\n",
        group_fd, pos_before, status, num_written, pos_after, read_buffer
    );

    fclose(group_fd);
    __memclr(read_buffer, READ_BUFFER_LEN);

    // ----------------------------------------------------

    fd_t libgdi_fd = fopen("/usr/lib/libgdi.so", O_RDWR, 0);
    for(uint16_t i = 0; i < 2048; i++) {
        num_written = fwrite(libgdi_fd, &i, sizeof(i), 0, &status);
        pos_after = fseek(libgdi_fd, 0, SEEK_CURR, NULL);
        printf(
            "Wrote %d to %s (fd %d): status %d, pos_after %d, num_written: %d\n",
            i, "/.../libgdi.so", libgdi_fd, status, pos_after, num_written
        );
        // sleep(25);
    }

    printf("Writing 2048 to libgdi.so\n");
    uint16_t overflow = 2048;
    pos_before = fseek(libgdi_fd, 0, SEEK_CURR, NULL);
    num_written = fwrite(libgdi_fd, &overflow, sizeof(overflow), 0, &status);
    pos_after = fseek(libgdi_fd, 0, SEEK_CURR, NULL);
    printf(
        "libgdi_fd: %d, pos_before %d, status: %d, num_written: %d, pos_after: %d\n",
        libgdi_fd, pos_before, status, num_written, pos_after
    );
    printf("Sleeping for 10 seconds so output can be read\n");
    sleep(10000);
    printf("Good morning Krusty Krew!\n");

    fseek(libgdi_fd, 0, SEEK_SET, NULL);
    for(int i = 0; i < 2048; i++) {
        fread(libgdi_fd, read_buffer, sizeof(uint16_t), 0, NULL);
        printf("Read value %d from /usr/lib/libgdi.so\n", *((uint16_t *) read_buffer));
        // sleep(25);
        sleep(10);
    }

    fclose(libgdi_fd);

    printf("Goodbye world!\n");
}

#define DENT_BUFFER_LEN 10

void __dump_children(fd_t fd)
{
    adinfs_dent_t dent_buffer[DENT_BUFFER_LEN];
    __memclr(dent_buffer, DENT_BUFFER_LEN * sizeof(adinfs_dent_t));

    int32_t status = 0;
    uint32_t num_children = flistdir(fd, NULL, 0, &status, 0);
    uint32_t num_read = flistdir(fd, dent_buffer, 256, &status, 0);
    printf(
        "fd: %d, num_children: %d, num_read: %d, status: %d\n",
        fd, num_children, num_read, status
    );

    for(uint32_t i = 0; i < num_read; i++) {
        printf(
            "child name: \"%s\", child_type: %d\n",
            dent_buffer[i].name, (dent_buffer[i].type == S_TYPE_FILE ? "file" : "dir")
        );
    }
}

void test_listdir(void)
{
    fd_t root_fd = fopen("/", O_READ, 0);
    fd_t etc_fd  = fopen("/etc", O_READ, 0);
    fd_t usr_fd  = fopen("/usr", O_READ, 0);
    fd_t lib_fd  = fopen("/usr/lib", O_READ, 0);
    fd_t bin_fd  = fopen("/usr/bin", O_READ, 0);

    __dump_children(root_fd);
    cwrites("\n");

    __dump_children(etc_fd);
    cwrites("\n");

    __dump_children(usr_fd);
    cwrites("\n");

    __dump_children(lib_fd);
    cwrites("\n");

    __dump_children(bin_fd);
}

void test_relative_paths(void)
{
    fd_t fd0 = fopen("..", O_READ, 0);
    __dump_children(fd0);
    cwrites("\n");

    sleep(5000);

    fd_t fd1 = fopen(".", O_READ, 0);
    __dump_children(fd1);
    cwrites("\n");

    sleep(5000);

    fd_t fd2 = fopen("etc/.", O_READ, 0);
    __dump_children(fd2);
    cwrites("\n");

    sleep(5000);

    fd_t fd3 = fopen("./etc/", O_READ, 0);
    __dump_children(fd3);

    fclose(fd0);
    fclose(fd1);
    fclose(fd2);
    fclose(fd3);
}

USERMAIN(test_vfs)
{
    // cwrites("Hello world!\n");

    // test_fd_assignment();
    // test_rw_locks();
    // test_fioctl();
    // test_read();
    // test_fseek();
    // test_write();
    // test_listdir();
    test_relative_paths();

    return 0;
}