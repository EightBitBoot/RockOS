/*
** File:    vgatext.c
**
** Author:  Seth Teichman
**
** Contributor: 
**
** Description: VGA Text Attribute routines
**
**  This module implements an expanded set of output routines
**  for the console screen on the machines in the DSL.
**  Refer to the header file comments for complete details.
**
** Naming conventions:
**
**  System Callable functions have names beginning with the
**  characters "__vga_".  Kernel functions have names beginning
**  with "_". Userspace callable functions have names without a 
**  preceding underscore.
**
*/

#include "vga.h"
#include "vgatext.h"
#include "cio.h"
#include "common.h"

unsigned int vga_text_fg (unsigned int c) {
    return (c & 0x0F) << 8; //Ensure color is free of extra bits, bitshift to foreground
}

unsigned int vga_text_bg (unsigned int c) {
    return vga_text_fg(c) << 4; //Ensure color is free of extra bits and in the foreground, bitshift to background
}

unsigned int ansi_color_to_vga_color(unsigned int ansi_color) {
    /*
    ** Convert an ANSI FGCOLOR to a VGA Color
    ** 0 is not a supported input, as it is ambiguous
    */
    unsigned int color_modifier = ansi_color/10;
    unsigned int base_color = ansi_color%10;
    unsigned int vga_color;
    switch(base_color) {
        case 0:
            vga_color = VGA_TEXT_COLOR_BLACK;
            break;
        case 1:
            vga_color = VGA_TEXT_COLOR_RED;
            break;
        case 2:
            vga_color = VGA_TEXT_COLOR_GREEN;
            break;
        case 3:
            vga_color = VGA_TEXT_COLOR_ORANGE;
            break;
        case 4:
            vga_color = VGA_TEXT_COLOR_BLUE;
            break;
        case 5:
            vga_color = VGA_TEXT_COLOR_MAGENTA;
            break;
        case 6:
            vga_color = VGA_TEXT_COLOR_CYAN;
            break;
        default:
            vga_color = VGA_TEXT_COLOR_WHITE;
            break;
    }

    switch(color_modifier) {
        case 3:
            return vga_text_fg(vga_color);
        case 9:
            return vga_text_fg(vga_color+VGA_TEXT_COLOR_LIGHTEN);
        case 4:
            return vga_text_bg(vga_color);
        case 10:
            return vga_text_bg(vga_color+VGA_TEXT_COLOR_LIGHTEN);
        default:
            __cio_printf("__ansi_color_to_vga_color received invalid ANSI color: %d", ansi_color);
            return 0;
    }
}

char* parse_ansi_color_code(char *buf, int *result_color) {
    /*
    ** See https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#colors--graphics-mode
    ** Supported formats:
    ** \x1b[MODEmTEXT - applies MODE
    ** \x1b[MODE;FGCOLORmTEXT - applies MODE, then FGCOLOR
    ** \x1b[MODE;FGCOLOR;BGCOLORmTEXT - applies MODE, then FGCOLOR and BGCOLOR
    ** Supported values for MODE:
    ** - 0 (reset all modes (styles and colors))
    ** Supported values for FGCOLOR:
    ** - 0 (reset - equivalent to 37)
    ** - 30 (black)
    ** - 31 (red)
    ** - 32 (green)
    ** - 33 (orange)
    ** - 34 (blue)
    ** - 35 (magenta)
    ** - 36 (cyan)
    ** - 37 (light gray)
    ** - 90 (dark gray)
    ** - 91 (light red)
    ** - 92 (light green)
    ** - 93 (yellow)
    ** - 94 (light blue)
    ** - 95 (light magenta)
    ** - 96 (light cyan)
    ** - 97 (white)
    ** Supported values for BGCOLOR:
    ** - 0 (reset - equivalent to 47)
    ** - 40 (black)
    ** - 41 (red)
    ** - 42 (green)
    ** - 43 (orange)
    ** - 44 (blue)
    ** - 45 (magenta)
    ** - 46 (cyan)
    ** - 47 (light gray)
    ** - 100 (dark gray)
    ** - 101 (light red)
    ** - 102 (light green)
    ** - 103 (yellow)
    ** - 104 (light blue)
    ** - 105 (light magenta)
    ** - 106 (light cyan)
    ** - 107 (white)
    **
    ** Takes in input pointer, returns pointer to end sentinel character (or NULL if invalid code)
    */
    int ch;
    unsigned int fgcolor, bgcolor;
    
    // [ Sentinel
    ch = *buf++;
    if (ch != '[')
        return 0; // end early if [ sentinel not present
   
    // MODE
    ch = *buf++;
    if (ch != '0') // our ANSI escape sequence implementation currently only supports resetting all modes each call
        return 0; // end early if not valid mode
    
    // ; or m Sentinel
    ch = *buf++;
    if (ch == 'm') {
        *result_color = VGA_TEXT_DEFAULT_COLOR_BYTE;
        return buf; // m sentinel is end of escape sequence, return
    } else if (ch == ';') {
        ch = *buf++; // ; sentinel means there's at least an FGCOLOR, get next char
    } else {
        return 0; // end early if m or ; sentinel not present
    }
    
    // FGCOLOR
    if (ch == '0') {
        fgcolor = 37;
    } else if (ch == '3') {
        fgcolor = 30;
    } else if (ch == '9') {
        fgcolor = 90;
    } else {
        return 0; // end early if invalid FGCOLOR starting character
    }
    ch = *buf++;
    if (fgcolor > 0) { // we have another digit to process
        switch(ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                fgcolor += ch - '0';
                break;
            default:
                return 0; // end early if invalid FGCOLOR second character
        }
    }

    // ; or m Sentinel
    ch = *buf++;
    if (ch == 'm') {
        *result_color = VGA_TEXT_DEFAULT_COLOR_BYTE;
        *result_color &= VGA_TEXT_COLOR_FG_CLEAR_MASK;
        *result_color |= ansi_color_to_vga_color(fgcolor);
        return buf; // m sentinel is end of escape sequence, return
    } else if (ch == ';') {
        ch = *buf++; // ; sentinel means there's a BGCOLOR, get next char
    } else {
        return 0; // end early if m or ; sentinel not present
    }

    //BGCOLOR
    if (ch == '0') {
        bgcolor = 0;
    } else if (ch == '4') {
        bgcolor = 40;
    } else if (ch == '1') {
        ch = *buf++;
        if (ch != '0')
            return 0; // end early if invalid BGCOLOR second character
        bgcolor = 100;
    } else {
        return 0; // end early if invalid BGCOLOR starting character
    }
    ch = *buf++;
    if (bgcolor > 0) { // we have another digit to process
        switch(ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                bgcolor += ch - '0';
                break;
            default:
                return 0; // end early if invalid BGCOLOR second character
        }
    }

    // m Sentinel
    ch = *buf++;
    if (ch != 'm')
        return 0; // end early if m sentinel not present

    *result_color = 0;
    if (fgcolor != 0) {
        *result_color |= ansi_color_to_vga_color(fgcolor);
    } else {
        *result_color |= vga_text_fg(VGA_TEXT_COLOR_WHITE);
    }
    if (bgcolor != 0) {
        *result_color |= ansi_color_to_vga_color(bgcolor);
    } else {
        *result_color |= vga_text_bg(VGA_TEXT_COLOR_BLACK);
    }

    return buf;
}

unsigned int __vga_text_get_active_color() {
    return active_color;
}

void __vga_text_set_active_color( unsigned int vga_text_color ) {
    active_color = vga_text_color;
}

unsigned int __vga_text_get_blink_enabled() {
    uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    return (attr_mode_ctl & 0b00001000) >> 3 == 1;
}

void __vga_text_set_blink_enabled(unsigned int blink_enabled) {
    uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    if (blink_enabled) {
        _vga_attr_write(VGA_ATTR_MODE_CTL, attr_mode_ctl | 0b00001000);
    } else {
        _vga_attr_write(VGA_ATTR_MODE_CTL, attr_mode_ctl & 0b11110111);   
    }
}

void _vga_text_init( void ) {
    // Set to Default Color Byte to finish Initialization
    active_color = VGA_TEXT_DEFAULT_COLOR_BYTE;
    __vga_text_set_blink_enabled(0);
}

void _vga_text_color_test( unsigned int kb_data, unsigned int kb_val ) {
    char* color_name;
    unsigned char state[VGA_NUM_REGS];
    uint8_t blink = __vga_text_get_blink_enabled();
    switch (kb_val) {
        case 0x60: // Backtick
            active_color = VGA_TEXT_DEFAULT_COLOR_BYTE;
            __cio_printf("[VGA] text color test: default text\n");    
            break;
        // Base Foreground Colors
        case 0x31: // 1
            active_color = vga_text_fg(VGA_TEXT_COLOR_BLACK);
            __cio_printf("[VGA] text color test: (not) black text\n");
            break;
        case 0x32: // 2
            active_color = vga_text_fg(VGA_TEXT_COLOR_BLUE);
            __cio_printf("[VGA] text color test: blue text\n");
            break;
        case 0x33: // 3
            active_color = vga_text_fg(VGA_TEXT_COLOR_GREEN);
            __cio_printf("[VGA] text color test: green text\n");
            break;
        case 0x34: // 4
            active_color = vga_text_fg(VGA_TEXT_COLOR_CYAN);
            __cio_printf("[VGA] text color test: cyan text\n");
            break;
        case 0x35: // 5
            active_color = vga_text_fg(VGA_TEXT_COLOR_RED);
            __cio_printf("[VGA] text color test: red text\n");
            break;
        case 0x36: // 6
            active_color = vga_text_fg(VGA_TEXT_COLOR_MAGENTA);
            __cio_printf("[VGA] text color test: magenta text\n");
            break;
        case 0x37: // 7
            active_color = vga_text_fg(VGA_TEXT_COLOR_ORANGE);
            __cio_printf("[VGA] text color test: orange text\n");
            break;
        case 0x38: // 8
            active_color = vga_text_fg(VGA_TEXT_COLOR_WHITE);
            __cio_printf("[VGA] text color test: white text\n");
            break;
        // Bright Foreground Colors
        case 0x71: // q
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_GRAY);
            __cio_printf("[VGA] text color test: (l) gray text\n");
            break;
        case 0x77: // w
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_BLUE);
            __cio_printf("[VGA] text color test: (l) light blue text\n");
            break;
        case 0x65: // e
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_GREEN);
            __cio_printf("[VGA] text color test: (l) light green text\n");
            break;
        case 0x72: // r
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_CYAN);
            __cio_printf("[VGA] text color test: (l) light cyan text\n");
            break;
        case 0x74: // t
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_RED);
            __cio_printf("[VGA] text color test: (l) light red text\n");
            break;
        case 0x79: // y
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_LIGHT_MAGENTA);
            __cio_printf("[VGA] text color test: (l) light magenta text\n");
            break;
        case 0x75: // u
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_YELLOW);
            __cio_printf("[VGA] text color test: (l) yellow text\n");
            break;
        case 0x69: // i
            active_color = vga_text_fg(VGA_TEXT_COLOR_L_WHITE_INTENSE);
            __cio_printf("[VGA] text color test: (l) white (intense) text\n");
            break;
        // Base Background Colors
        case 0x61: // a
            active_color = vga_text_bg(VGA_TEXT_COLOR_BLACK);
            __cio_printf("[VGA] text color test: black background\n");
            break;
        case 0x73: // s
            active_color = vga_text_bg(VGA_TEXT_COLOR_BLUE);
            __cio_printf("[VGA] text color test: blue background\n");
            break;
        case 0x64: // d
            active_color = vga_text_bg(VGA_TEXT_COLOR_GREEN);
            __cio_printf("[VGA] text color test: green background\n");
            break;
        case 0x66: // f
            active_color = vga_text_bg(VGA_TEXT_COLOR_CYAN);
            __cio_printf("[VGA] text color test: cyan background\n");
            break;
        case 0x67: // g
            active_color = vga_text_bg(VGA_TEXT_COLOR_RED);
            __cio_printf("[VGA] text color test: red background\n");
            break;
        case 0x68: // h
            active_color = vga_text_bg(VGA_TEXT_COLOR_MAGENTA);
            __cio_printf("[VGA] text color test: magenta background\n");
            break;
        case 0x6a: // j
            active_color = vga_text_bg(VGA_TEXT_COLOR_ORANGE);
            __cio_printf("[VGA] text color test: orange background\n");
            break;
        case 0x6b: // k
            active_color = vga_text_bg(VGA_TEXT_COLOR_WHITE);
            __cio_printf("[VGA] text color test: white background\n");
            break;
        // Bright/Blink Background Colors
        case 0x7a: // z
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_BLACK);
            color_name = blink ? "blink text + black" : "gray";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x78: // x
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_BLUE);
            color_name = blink ? "blink text + blue" : "light blue";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x63: // c
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GREEN);
            color_name = blink ? "blink text + green" : "light green";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x76: // v
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_CYAN);
            color_name = blink ? "blink text + cyan" : "light cyan";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x62: // b
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_RED);
            color_name = blink ? "blink text + red" : "light red";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x6e: // n
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_MAGENTA);
            color_name = blink ? "blink text + magenta" : "light magenta";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x6d: // m
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_ORANGE);
            color_name = blink ? "blink text + orange" : "yellow";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x2c: // ,
            active_color = vga_text_bg(VGA_TEXT_COLOR_BG_BLINK_GRAY);
            color_name = blink ? "blink text + white" : "white (high intensity)";
            __cio_printf("[VGA] text color test: (b) %s background\n", color_name);
            break;
        case 0x2f: // /
            // Disable Blink to gain more Background Colors
            if (__vga_text_get_blink_enabled()) {
                __vga_text_set_blink_enabled(0);
            } else {
                __vga_text_set_blink_enabled(1);
            }
            break;
        case 0x2e: // .
            switch (__vga_get_mode()) {
                case 0:
                    __vga_set_mode(1);
                    break;
                case 1:
                    __vga_set_mode(2);
                    break;
                case 2:
                    __vga_set_mode(0);
                    break;
            }
            break;
        case 0x0a: // Enter
            _read_regs(state);
            _dump_regs(state);
            break;
        case 0x2d: // -: Clear Screen (SLOW)
            __vga_clear_screen();
            break;
        case 0x3d: // =: Draw blue/green X
            _draw_x();
            break;
        case 0x5c: // \: Draw Gradient
            __vga_draw_test_pattern();
            break;
        case 0x08: // Backspace: Draw Rick Astley
            __vga_draw_image(320, 180, 0, 0, vga_image_rick);
            break;
        case 0x5b: // [: Draw Baby Adin
            __vga_draw_image(157, 180, 0, 0, vga_image_adin);
            break;
        case 0x5d: // ]: Draw Obi-Wan
            __vga_draw_image(320, 135, 0, 0, vga_image_obiwan);
            break;
        case 0x27: // ': Draw Coyote
            __vga_draw_image(280, 200, 20, 0, vga_image_coyote);
            break;
    }
    active_color = VGA_TEXT_DEFAULT_COLOR_BYTE;
}

char logo[432] = {
    ' ',' ',' ',0x2f,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,'\n',
' ',' ',' ',0x5c,' ',' ',' ',0x2f,' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,'\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ','\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x5c,0x3d,0x3d,0x2f,' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,'\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x7c,' ',' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x7c,'\n',
0x2f,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,'\n',
0x7c,' ',0x5c,0x3d,0x2f,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x5c,0x3d,0x3d,0x2f,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x7c,'\n',
0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',0x5c,0x3d,0x2f,' ',' ',0x5c,0x3d,0x2f,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,
};