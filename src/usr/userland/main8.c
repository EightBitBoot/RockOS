#ifndef MAIN_8_H_
#define MAIN_8_H_

#include "usr/users.h"
#include "usr/ulib.h"
#include "io/vga.h"
#include "io/vgatext.h"

/**
** User function main #8: VGA 16 Color Graphics mode color test
**
** Prints each possible color in a vertical bar
**
** Invoked as:  main8
*/

#define half 5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
#define S1 -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
#define S2 8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
#define S3 -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
#define S4 2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
#define S5 -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
#define S6 1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

#define one  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
#define C1   4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
#define C2  -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
#define C3   2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
#define C4  -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
#define C5   2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
#define C6  -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

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

char logo[432] = {
    ' ',' ',' ',0x2f,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x5c,'\n',
' ',' ',' ',0x5c,' ',' ',' ',0x2f,' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,'\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ','\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x5c,0x3d,0x3d,0x2f,' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x5c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,'\n',
' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x2f,0x3d,0x3d,0x5c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x7c,' ',' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x5c,' ',0x7c,'\n',
0x2f,0x3d,0x5c,' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',' ',' ',' ',' ',0x7c,' ',0x7c,'\n',
0x7c,' ',0x5c,0x3d,0x2f,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',' ',0x7c,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x7c,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x7c,' ',0x5c,0x3d,0x3d,0x2f,' ',0x7c,' ',0x2f,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x7c,'\n',
0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',0x5c,0x3d,0x2f,' ',' ',0x5c,0x3d,0x2f,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,' ',0x5c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x2f,'\n',
};

USERMAIN( main8 ) {
    int c;
    // Enter 16 Color Graphics Mode
    vgasetmode(1);

    // Clear Screen
    vgaclearscreen();

	// Print Color Tests
    // vgatest();

    // sleep(1000);

    vgaclearscreen();

    // Draw some Squares
    for (c = 0; c < 16; c++) {
        draw_square((640/2)+(c-8)*25, (480/2)+(c-8)*25, 20, c);
    }

    sleep(2500);

    // Enter 256 Color Graphics Mode
    vgasetmode(2);

    // Clear Screen
    vgaclearscreen();

	// Print Color Tests
    vgatest();

    sleep(1000);

    vgaclearscreen();

    // Clear Screen Again
    vgaclearscreen();

    vgadrawimage(320, 180, 0, 10, vga_image_rick);

    sleep(2500);

    vgaclearscreen();

    vgadrawimage(320, 135, 0, 32, vga_image_obiwan);

    sleep(2500);

    vgaclearscreen();

    vgadrawimage(157, 180, 81, 10, vga_image_adin);

    sleep(2500);

    vgaclearscreen();

    vgadrawimage(280, 200, 20, 0, vga_image_coyote);

    sleep(5000);

    // Return to Text Mode
    vgasetmode(0);

    vgatextclear();

    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_ORANGE));

    cwrites(logo);

    cwritech('\n');

    vgatextsetactivecolor(vga_text_fg(VGA_TEXT_COLOR_BLACK) | vga_text_bg(VGA_TEXT_COLOR_WHITE));

    cwrites("          Jake, Adin and Seth OS Version 1           \n");

    vgatextsetactivecolor(VGA_TEXT_DEFAULT_COLOR_BYTE);

    cwrites("\n\n_____________________________________________________");

    cwrites("\n\n\n\n\n");

    sleep(5000);

	// all done!
	exit( 0 );

    return 0;
}

#endif
