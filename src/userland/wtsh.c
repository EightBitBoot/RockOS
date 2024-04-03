
#include "users.h"
#include "ulib.h"
#include "common.h"

// Compile Time Settings
#define WTSH_HIST_SIZE (20)
#define LINE_BUFFER_SIZE (80) // 80 cols NOT null terminated

#define INTERNAL_COMMAND USERMAIN

// Forward declerations of internal commands
INTERNAL_COMMAND(int_cmd_echo);
INTERNAL_COMMAND(int_cmd_exit);

typedef struct command_entry
{
    char name[128]; // Arbitrary size but makes the memory usage consistent
    userfcn_t entrypoint;
    uint8_t is_subprocess; // Does the command require a fork or is it internal?
} command_entry_t;

#define COMMAND_ENTRY(name, entry, is_subprocess) { (name), (entry), (is_subprocess) }

command_entry_t g_commands[] = {
    COMMAND_ENTRY("echo", int_cmd_echo, 0),
    COMMAND_ENTRY("exit", int_cmd_exit, 0)
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
    char curr_char = 0;
    char line_buffer[LINE_BUFFER_SIZE] = {};
    uint8_t cursor_pos = 0;

    char print_buffer[256];

    init_state(&g_shell_state);

    cwrites("wtsh> ");

    while(g_shell_state.is_running) {
        // // I don't like busy loops but that's what happens when you have non-blocking i/o
        // while(read(CHAN_CIO, &curr_char, 1) == E_NO_DATA) {}
        // sprint(print_buffer, "char: %c\n", curr_char);

        int32_t num_read = read(CHAN_CIO, &curr_char, 1);

        if(curr_char != 0) {
            sprint(print_buffer, "num_read: %d, curr_char: %c\n", num_read, curr_char);
        }
        else {
            sprint(print_buffer, "num_read: %d, curr_char: NULL\n", num_read);
        }
        cwrites(print_buffer);

        if(IS_PRINTABLE(curr_char)) {
            line_buffer[cursor_pos] = curr_char;
            if(cursor_pos < LINE_BUFFER_SIZE - 1) {
                cursor_pos++;
            }
            write(CHAN_CIO, line_buffer, cursor_pos + 1); // Do I need to manually echo?
        }
    }

    cwrites("Oh no they took me away!\n");

    return 0;
}

// -- Internal Command Entrypoints --

INTERNAL_COMMAND(int_cmd_echo)
{
    return 0;
}

INTERNAL_COMMAND(int_cmd_exit)
{
    g_shell_state.is_running = false;

    return 0;
}