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
#define VGA_TEXT_COLOR_BROWN 0x6
#define VGA_TEXT_COLOR_GRAY 0x7
// Bright Color Attributes
#define VGA_TEXT_COLOR_BRIGHTEN 0x8
#define VGA_TEXT_COLOR_L_DARK_GRAY VGA_TEXT_COLOR_BLACK+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_BLUE VGA_TEXT_COLOR_BLUE+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_GREEN VGA_TEXT_COLOR_GREEN+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_CYAN VGA_TEXT_COLOR_CYAN+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_RED VGA_TEXT_COLOR_RED+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_LIGHT_MAGENTA VGA_TEXT_COLOR_MAGENTA+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_YELLOW VGA_TEXT_COLOR_BROWN+VGA_TEXT_COLOR_BRIGHTEN
#define VGA_TEXT_COLOR_L_WHITE VGA_TEXT_COLOR_GRAY+VGA_TEXT_COLOR_BRIGHTEN
// Default Color
#define VGA_TEXT_DEFAULT_COLOR_BYTE 0x700
// Active Color
static unsigned int    active_color; // The color to be masked onto the character - as such, the lower 8 bits should be 0 and the upper 8 should store the VGA color data

// Utility Functions
unsigned int vga_text_fg (unsigned int c);
unsigned int vga_text_bg (unsigned int c);
static char* parse_ansi_color_code(char *buf, int *result_color);
unsigned int ansi_color_to_vga_color(unsigned int ansi_color);
// System Calls
unsigned int __vga_text_get_active_color();
void __vga_text_set_active_color(unsigned int vga_text_color);

// Initialization Function
void __vga_text_init(void);
#endif