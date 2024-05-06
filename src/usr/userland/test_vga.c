#ifndef MAIN_7_H_
#define MAIN_7_H_

#include "usr/users.h"
#include "usr/ulib.h"
#include "io/vgatext.h"

/**
** User function main #7: VGA text mode color test
**
** Prints each possible color text by name in its color
**
** Invoked as:  main7
*/

USERMAIN( test_vga ) {
	vgatextclear();

	// Print Color Tests: Foreground Color
    cwrites("main7: default text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLACK));
    cwrites("main7: black text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLUE));
    cwrites("main7: blue text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_GREEN));
    cwrites("main7: green text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_CYAN));
    cwrites("main7: cyan text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_RED));
    cwrites("main7: red text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_MAGENTA));
    cwrites("main7: magenta text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_ORANGE));
    cwrites("main7: orange text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_WHITE));
    cwrites("main7: white text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_GRAY));
    cwrites("main7: (l) gray text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_BLUE));
    cwrites("main7: (l) light blue text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_GREEN));
    cwrites("main7: (l) light green text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_CYAN));
    cwrites("main7: (l) light cyan text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_RED));
    cwrites("main7: (l) light red text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_MAGENTA));
    cwrites("main7: (l) light magenta text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_YELLOW));
    cwrites("main7: (l) yellow text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_L_WHITE_INTENSE));
    cwrites("main7: (l) white (high intensity) text\n");
    // Print Color Tests: Background Color
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BLACK));
    cwrites("main7: black background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BLUE));
    cwrites("main7: blue background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_GREEN));
    cwrites("main7: green background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_CYAN));
    cwrites("main7: cyan background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_RED));
    cwrites("main7: red background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_MAGENTA));
    cwrites("main7: magenta background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_ORANGE));
    cwrites("main7: orange background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_WHITE));
    cwrites("main7: white background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_L_GRAY));
    cwrites("main7: (l) gray background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_L_LIGHT_BLUE));
    cwrites("main7: (l) light blue background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GREEN));
    cwrites("main7: (l) light green background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_CYAN));
    cwrites("main7: (l) light cyan background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_RED));
    cwrites("main7: (l) light red background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_MAGENTA));
    cwrites("main7: (l) light magenta background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_ORANGE));
    cwrites("main7: (l) yellow background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GRAY));
    cwrites("main7: (l) white (high intensity) background\n");
    vgatextsetactivecolor(VGA_TEXT_DEFAULT_COLOR_BYTE);

	// all done!
	exit( 0 );

    return 0;
}

#endif
