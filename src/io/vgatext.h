/*
** File:    vgatext.h
**
** Author:  Seth Teichman
**
** Contributor: 
**
** Description: VGA Text Attribute routines
**
**  This module implements an expanded set of output routines
**  for the console screen on the machines in the DSL.
**
** Naming conventions:
**
**  System Callable functions have names beginning with the
**  characters "__vga_".  Kernel functions have names beginning
**  with "_". Userspace callable functions have names without a 
**  preceding underscore.
**
**  Major functions are:
**  Get/Set Color
**  Clear Screen
**  Get/Set Text Blink xor 16-color Backgrounds
**  ANSI Color Code Parsing for use in printf Implementations
**
*/

#ifndef VGATEXT_H
#define VGATEXT_H
// Utility Attributes
#define VGA_TEXT_COLOR_BG_CLEAR_MASK 0b0000111111111111
#define VGA_TEXT_COLOR_FG_CLEAR_MASK 0b1111000011111111
// Base Color Attributes
#define VGA_TEXT_COLOR_BLACK 0x0
#define VGA_TEXT_COLOR_BLUE 0x1
#define VGA_TEXT_COLOR_GREEN 0x2
#define VGA_TEXT_COLOR_CYAN 0x3
#define VGA_TEXT_COLOR_RED 0x4
#define VGA_TEXT_COLOR_MAGENTA 0x5
#define VGA_TEXT_COLOR_ORANGE 0x6
#define VGA_TEXT_COLOR_BROWN VGA_TEXT_COLOR_ORANGE // VGA Spec refers to this color as Brown
#define VGA_TEXT_COLOR_WHITE 0x7
// Bright Color Attributes for Foreground, and for Background out of Blink Mode
#define VGA_TEXT_COLOR_LIGHTEN 0x8
#define VGA_TEXT_COLOR_L_GRAY VGA_TEXT_COLOR_BLACK+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_BLUE VGA_TEXT_COLOR_BLUE+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_GREEN VGA_TEXT_COLOR_GREEN+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_CYAN VGA_TEXT_COLOR_CYAN+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_RED VGA_TEXT_COLOR_RED+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_MAGENTA VGA_TEXT_COLOR_MAGENTA+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_YELLOW VGA_TEXT_COLOR_ORANGE+VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_L_WHITE_INTENSE VGA_TEXT_COLOR_WHITE+VGA_TEXT_COLOR_LIGHTEN
// Blink Color Attributes for Background in Blink Mode
#define VGA_TEXT_COLOR_BG_BLINKEN VGA_TEXT_COLOR_LIGHTEN
#define VGA_TEXT_COLOR_BG_BLINK_BLACK VGA_TEXT_COLOR_L_GRAY
#define VGA_TEXT_COLOR_BG_BLINK_BLUE VGA_TEXT_COLOR_L_LIGHT_BLUE
#define VGA_TEXT_COLOR_BG_BLINK_GREEN VGA_TEXT_COLOR_L_LIGHT_GREEN
#define VGA_TEXT_COLOR_BG_BLINK_CYAN VGA_TEXT_COLOR_L_LIGHT_CYAN
#define VGA_TEXT_COLOR_BG_BLINK_RED VGA_TEXT_COLOR_L_LIGHT_RED
#define VGA_TEXT_COLOR_BG_BLINK_MAGENTA VGA_TEXT_COLOR_L_LIGHT_MAGENTA
#define VGA_TEXT_COLOR_BG_BLINK_ORANGE VGA_TEXT_COLOR_L_YELLOW
#define VGA_TEXT_COLOR_BG_BLINK_GRAY VGA_TEXT_COLOR_L_WHITE_INTENSE
// Default Color
#define VGA_TEXT_DEFAULT_COLOR_BYTE 0x700
// Active Color
unsigned int    active_color; // The color to be masked onto the character - as such, the lower 8 bits should be 0 and the upper 8 should store the VGA color data
// Logo Blob
extern char logo[432];

// Utility Functions
unsigned int vga_text_fg (unsigned int c);
unsigned int vga_text_bg (unsigned int c);
char* parse_ansi_color_code(char *buf, int *result_color);
unsigned int ansi_color_to_vga_color(unsigned int ansi_color);
// System Calls
unsigned int __vga_text_get_active_color(void);
void __vga_text_set_active_color(unsigned int vga_text_color);
unsigned int __vga_text_get_blink_enabled(void);
void __vga_text_set_blink_enabled(unsigned int blink_enabled);

// Initialization Function
void _vga_text_init(void);
// Test Function
void _vga_text_color_test( unsigned int kb_data, unsigned int kb_val );
#endif
