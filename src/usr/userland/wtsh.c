
#include "common.h"
#include "usr/users.h"
#include "usr/ulib.h"
#include "io/vga.h"
#include "io/vgatext.h"

#define ARRAY_LEN(array) (sizeof((array)) / sizeof(*(array)))

// Compile Time Settings
#define WTSH_HIST_SIZE (20)
#define LINE_BUFFER_SIZE (80) // 80 cols NOT null terminated

#define INTERNAL_COMMAND USERMAIN

// Forward declarations of internal commands
INTERNAL_COMMAND(int_cmd_help);
INTERNAL_COMMAND(int_cmd_echo);
INTERNAL_COMMAND(int_cmd_exit);
INTERNAL_COMMAND(int_cmd_shutdown);
INTERNAL_COMMAND(int_cmd_reboot);
INTERNAL_COMMAND(int_cmd_ls);
INTERNAL_COMMAND(int_cmd_vgademo);
INTERNAL_COMMAND(int_cmd_getcwd);
INTERNAL_COMMAND(int_cmd_cd);

#define COMMAND_STR_SIZE (128)
typedef struct command_entry
{
    char name[COMMAND_STR_SIZE]; // Arbitrary size but makes the memory usage consistent
    char description[COMMAND_STR_SIZE]; // Used by help command to show description
    userfcn_t entrypoint;
    uint8_t is_subprocess; // Does the command require a fork or is it internal?
} command_entry_t;

#define COMMAND_ENTRY(name, description, entry, is_subprocess) { (name), (description), (entry), (is_subprocess) }

command_entry_t g_commands[] = {
    COMMAND_ENTRY("help", "display this help text", int_cmd_help, 0),
    COMMAND_ENTRY("echo", "output a line of text", int_cmd_echo, 0),
    COMMAND_ENTRY("exit", "exit the shell", int_cmd_exit, 0),
    COMMAND_ENTRY("shutdown", "shutdown the system", int_cmd_shutdown, 0),
    COMMAND_ENTRY("reboot", "reboot the system", int_cmd_reboot, 0),
    COMMAND_ENTRY("ls", "list directory contents", int_cmd_ls, 0),
    COMMAND_ENTRY("vgademo", "demonstrate vga", int_cmd_vgademo, 0),
    COMMAND_ENTRY("pwd", "print the current working directory of the shell", int_cmd_getcwd, 0),
    COMMAND_ENTRY("cd", "change the current working directory of the shell", int_cmd_cd, 0),

    COMMAND_ENTRY("test_vfs", "run various userspace vfs tests", test_vfs, 1),
    {}, // End sentinel (ensures there's always an element in the array for sizing)
};

typedef struct wtsh_state
{
    bool_t is_running;
} wtsh_state_t;

static wtsh_state_t g_shell_state;

void init_state(wtsh_state_t *state)
{
    state->is_running = true;
}

#define IS_PRINTABLE(c) (0x20 <= (c) && (c) <= 0x7E)

// Shell entry point
USERMAIN(wtsh_main)
{
    char in_buf[2] = {};
    char line_buffer[LINE_BUFFER_SIZE] = {};
    char out[32] = {};
    int x, y = 0;
    bool_t cmd_found = 0;
    uint8_t cursor_pos = 0;

    init_state(&g_shell_state);

    vgatextclear();

    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_ORANGE));

    cwrites(logo);

    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLACK) | vga_text_bg(VGA_TEXT_COLOR_WHITE));

    cwrites("\n\n");

    cwrites("          Jake, Adin and Seth OS Version 1           \n");

    vgatextsetactivecolor(VGA_TEXT_DEFAULT_COLOR_BYTE);

    cwrites("\n\n_____________________________________________________");

    cwrites("\n\n\n\n\n");

    cwrites("wtsh> ");
    while(g_shell_state.is_running) {
        // I don't like busy loops but that's what happens when you have non-blocking i/o
        while(read(CHAN_CIO, in_buf, 2) == E_NO_DATA) {}

	    char c = in_buf[0];
        if (ciogetspecialdown()) {
            if (c == 0x34) { // Left arrow
                if (cursor_pos > 0) {
                    ciogetcursorpos(&x, &y);
                    ciosetcursorpos(x-1, y);
                    cursor_pos--;
                }
            } else if (c == 0x36) { // Right arrow
                if (line_buffer[cursor_pos] != '\0') {
                    ciogetcursorpos(&x, &y);
                    ciosetcursorpos(x+1, y);
                    cursor_pos++;
                }
            } else {
                // sprint(out, "Unprintable Input: %02x\n", c);
                // cwrites(out);
            }
            in_buf[0] = 0;
        } else if(c == 0x0A) { // Enter key
                line_buffer[cursor_pos] = '\0';
                cwritech('\n');
                cmd_found = 0;
                // todo: do the thing
                for (uint8_t i = 0; i < ARRAY_LEN(g_commands); i++) {
                    command_entry_t cmd = g_commands[i];

                    if (strcmp(cmd.name, line_buffer) == 0) {
                        cmd_found = 1;
                        if(cmd.is_subprocess) {
                            int32_t pid = spawn(cmd.entrypoint, -1, NULL);
                            waitpid(pid, NULL); // TODO: keep track of status? expose as variable or output it after?
                        }
                        else {
                            cmd.entrypoint(0, NULL);
                        }
                        break;
                    }
                }

                if (!cmd_found && line_buffer[0] != '\0') {
                    sprint(out, "Command '%s' not found!\n", line_buffer);
                    cwrites(out);
                }

                

                in_buf[0] = 0;
                cursor_pos = 0;
                for (uint8_t i = 0; i < LINE_BUFFER_SIZE; i++) {
                    line_buffer[i] = '\0';
                }
                cwrites("wtsh> ");
        } else if (c == 0x08) { // Backspace
            if (cursor_pos > 0) {
                cursor_pos--;
                line_buffer[cursor_pos] = '\0';

                ciogetcursorpos(&x, &y);
                ciosetcursorpos(x-1, y);
                cwritech(' ');
                ciosetcursorpos(x-1, y);
            }
        } else if (IS_PRINTABLE(c)) {
            line_buffer[cursor_pos] = c;
            cwritech(c);

            if(cursor_pos < LINE_BUFFER_SIZE - 1) {
                cursor_pos++;
            }
        }
    }

    cwrites("Oh no they took me away!\n");

    return 0;
}

// -- Internal Command Entrypoints --

INTERNAL_COMMAND(int_cmd_help)
{
    char buffer[(COMMAND_STR_SIZE * 2) + 9]; // calculated based on command entry size
    sprint(buffer, "wtsh v1.0 (%s %s)\n\n", __DATE__, __TIME__);
    cwrites(buffer);

    for (uint8_t i = 0; i < ARRAY_LEN(g_commands); i++) {
        command_entry_t cmd = g_commands[i];

        if (IS_PRINTABLE(cmd.name[0])) {
            sprint(buffer, "    %-12s - %s\n", cmd.name, cmd.description);
            cwrites(buffer);
        }
    }

    cwrites("\n");

    return 0;
}

INTERNAL_COMMAND(int_cmd_echo)
{
    return 0;
}

INTERNAL_COMMAND(int_cmd_exit)
{
    g_shell_state.is_running = false;

    return 0;
}

/* ACPI */
#include <acpi/acpi.h>
INTERNAL_COMMAND(int_cmd_shutdown)
{
    cwrites("Broadcast message\n\n");
    cwrites("The system is going down for shutdown NOW!\n");

    acpicommand(ACPI_COMMAND_SHUTDOWN);

    return 0;
}

INTERNAL_COMMAND(int_cmd_reboot)
{
    cwrites("Broadcast message\n\n");
    cwrites("The system is going down for reboot NOW!\n");

    acpicommand(ACPI_COMMAND_REBOOT);

    return 0;
}

INTERNAL_COMMAND(int_cmd_ls)
{
    return 0;
}

void draw_square(unsigned x, unsigned y, unsigned side, unsigned color) {
    uint16_t cx=x, cy=y;
    for (cx = x; cx < x+side; cx++) {
        vgawritepixel(cx, cy, color);
    }
    for (cy = y; cy < y+side; cy++) {
        vgawritepixel(cx, cy, color+16);
    }
    for (cx = x+side; cx > x; cx--) {
        vgawritepixel(cx, cy, color+32);
    }
    for (cy = y+side; cy > y; cy--) {
        vgawritepixel(cx, cy, color+48);
    }
}

INTERNAL_COMMAND(int_cmd_vgademo)
{
    int c;
    // Enter 16 Color Graphics Mode
    vgasetmode(1);

    // Clear Screen
    vgaclearscreen();

	// Print Color Tests
    vgatest();

    sleep(5000);

    vgaclearscreen();

    // Draw some Squares
    for (c = 0; c < 16; c++) {
        draw_square((640/2)+(c-8)*25, (480/2)+(c-8)*25, 20, c);
    }

    sleep(5000);

    // Enter 256 Color Graphics Mode
    vgasetmode(2);

    // Clear Screen
    vgaclearscreen();

	// Print Color Tests
    vgatest();

    sleep(5000);

    vgaclearscreen();

    // Clear Screen Again
    vgaclearscreen();

    vgadrawimage(320, 180, 0, 10, vga_image_rick);

    sleep(5000);

    vgaclearscreen();

    vgadrawimage(320, 135, 0, 32, vga_image_obiwan);

    sleep(5000);

    vgaclearscreen();

    vgadrawimage(157, 180, 81, 10, vga_image_adin);

    sleep(5000);

    vgaclearscreen();

    vgadrawimage(280, 200, 20, 0, vga_image_coyote);

    sleep(5000);

    // Return to Text Mode
    vgasetmode(0);

    vgatextclear();

    cwrites("wtsh> ");

	// all done!
    return 0;
}

#define CWD_BUFFER_LEN 1024
char cwd_buffer[CWD_BUFFER_LEN] = {0};

INTERNAL_COMMAND(int_cmd_getcwd)
{
    fgetcwd(cwd_buffer, CWD_BUFFER_LEN);
    cwrites(cwd_buffer);
    cwrites("\n");

    return 0;
}

INTERNAL_COMMAND(int_cmd_cd)
{
    return 0;
}

