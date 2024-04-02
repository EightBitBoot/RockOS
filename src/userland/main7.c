#ifndef MAIN_7_H_
#define MAIN_7_H_

#include "users.h"
#include "ulib.h"
#include "vgatext.h"

/**
** User function main #7: VGA text mode color test
**
** Prints each possible color text by name in its color
**
** Invoked as:  main7
*/

USERMAIN( main7 ) {
	vgatextclear();

	// Print Color Tests: Foreground Color
    cwrites("[VGA_T] init: default text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLACK));
    cwrites("[VGA_T] init: black text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLUE));
    cwrites("[VGA_T] init: blue text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_GREEN));
    cwrites("[VGA_T] init: green text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_CYAN));
    cwrites("[VGA_T] init: cyan text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_RED));
    cwrites("[VGA_T] init: red text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_MAGENTA));
    cwrites("[VGA_T] init: magenta text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_ORANGE));
    cwrites("[VGA_T] init: orange text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_GRAY));
    cwrites("[VGA_T] init: gray text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_DARK_GRAY));
    cwrites("[VGA_T] init: dark gray text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_LIGHT_BLUE));
    cwrites("[VGA_T] init: light blue text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_LIGHT_GREEN));
    cwrites("[VGA_T] init: light green text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_LIGHT_CYAN));
    cwrites("[VGA_T] init: light cyan text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_LIGHT_RED));
    cwrites("[VGA_T] init: light red text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_LIGHT_MAGENTA));
    cwrites("[VGA_T] init: light magenta text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_YELLOW));
    cwrites("[VGA_T] init: yellow text\n");
    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_FG_WHITE));
    cwrites("[VGA_T] init: white text\n");
    // Print Color Tests: Background Color
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BLACK));
    cwrites("[VGA_T] init: black background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BLUE));
    cwrites("[VGA_T] init: blue background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_GREEN));
    cwrites("[VGA_T] init: green background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_CYAN));
    cwrites("[VGA_T] init: cyan background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_RED));
    cwrites("[VGA_T] init: red background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_MAGENTA));
    cwrites("[VGA_T] init: magenta background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_ORANGE));
    cwrites("[VGA_T] init: orange background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_GRAY));
    cwrites("[VGA_T] init: gray background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_BLACK));
    cwrites("[VGA_T] init: blink text black background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_BLUE));
    cwrites("[VGA_T] init: blink text blue background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GREEN));
    cwrites("[VGA_T] init: blink text green background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_CYAN));
    cwrites("[VGA_T] init: blink text cyan background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_RED));
    cwrites("[VGA_T] init: blink text red background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_MAGENTA));
    cwrites("[VGA_T] init: blink text magenta background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_ORANGE));
    cwrites("[VGA_T] init: blink text orange background\n");
    vgatextsetactivecolor(vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GRAY));
    cwrites("[VGA_T] init: blink text gray background\n");
    vgatextsetactivecolor(VGA_TEXT_DEFAULT_COLOR_BYTE);

	// all done!
	exit( 0 );

    return 0;
}

#endif
