/*
** SCCS ID: @(#)cio.c	2.8        3/4/24
**
** File:    cio.c
**
** Author:  Warren R. Carithers
**
** Contributor: Seth Teichman
**
** Based on:    c_io.c 1.13 (Ken Reek, Jon Coles, Warren R. Carithers)
**
** Description: Console I/O routines
**
**  This module implements a simple set of input and output routines
**  for the console screen and keyboard on the machines in the DSL.
**  Refer to the header file comments for complete details.
**
** Naming conventions:
**
**  Externally-visible functions have names beginning with the
**  characters "__cio_".  Internal functions have names beginning
**  with "__c_".
**
*/

#include "cio.h"
#include "libc/lib.h"
#include "kern/support.h"
#include "x86arch.h"
#include "x86pic.h"
#include "vgatext.h"

/*
** Miscellaneous useful things
*/
#define BMASK5      0x1f
#define BMASK8      0xff

/*
** Video parameters, and state variables
*/
#define SCREEN_MIN_X    0
#define SCREEN_MIN_Y    0
#define SCREEN_X_SIZE   80
#define SCREEN_Y_SIZE   25
#define SCREEN_MAX_X    ( SCREEN_X_SIZE - 1 )
#define SCREEN_MAX_Y    ( SCREEN_Y_SIZE - 1 )

static unsigned int    scroll_min_x, scroll_min_y;
static unsigned int    scroll_max_x, scroll_max_y;
static unsigned int    curr_x, curr_y;
static unsigned int    min_x, min_y;
static unsigned int    max_x, max_y;
static unsigned int    arrow_pressed = 0;

// pointer to input notification function
static void (*__c_notify)(int);

#ifdef  SA_DEBUG
#include <stdio.h>
#define __cio_putchar   putchar
#define __cio_puts(x)   fputs( x, stdout )
#endif

#define VIDEO_ADDR(x,y) ( unsigned short * ) \
        ( VIDEO_BASE_ADDR + 2 * ( (y) * SCREEN_X_SIZE + (x) ) )

/*
** Support routines.
**
** __c_putchar_at: physical output to the video memory
** __c_setcursor: set the cursor location (screen coordinates)
*/
static void __c_setcursor( void ) {
    unsigned addr;
    unsigned int y = curr_y;

    if( y > scroll_max_y ) {
        y = scroll_max_y;
    }

    addr = (unsigned)( y * SCREEN_X_SIZE + curr_x );

    __outb( 0x3d4, 0xe );
    __outb( 0x3d5, ( addr >> 8 ) & BMASK8 );
    __outb( 0x3d4, 0xf );
    __outb( 0x3d5, addr & BMASK8 );
}

static void __c_putchar_at( unsigned int x, unsigned int y, unsigned int c ) {
    /*
    ** If x or y is too big or small, don't do any output.
    */
    if( x <= max_x && y <= max_y ) {
        unsigned short *addr = VIDEO_ADDR( x, y );

        if( c > BMASK8 ) {
            /*
            ** Use the given attributes
            */
            *addr = (unsigned short)c;
        } else {
            /*
            ** Use attributes 0000 0111 (white on black)
            */
            *addr = (unsigned short)c | 0x0700;
        }
    }
}

void __cio_setscroll( unsigned int s_min_x, unsigned int s_min_y,
                      unsigned int s_max_x, unsigned int s_max_y ) {
    scroll_min_x = __bound( min_x, s_min_x, max_x );
    scroll_min_y = __bound( min_y, s_min_y, max_y );
    scroll_max_x = __bound( scroll_min_x, s_max_x, max_x );
    scroll_max_y = __bound( scroll_min_y, s_max_y, max_y );
    curr_x = scroll_min_x;
    curr_y = scroll_min_y;
    __c_setcursor();
}

/*
** Retrieve current cursor position
*/
void __cio_getpos( unsigned int *x, unsigned int *y) {
    *x = curr_x;
    *y = curr_y;
}

/*
** Retrieve current arrow key pressed status
*/
unsigned int __cio_getarrowdown(void) {
    return arrow_pressed;
}

/*
** Cursor movement in the scroll region
*/
void __cio_moveto( unsigned int x, unsigned int y ) {
    curr_x = __bound( scroll_min_x, x + scroll_min_x, scroll_max_x );
    curr_y = __bound( scroll_min_y, y + scroll_min_y, scroll_max_y );
    __c_setcursor();
}

/*
** The putchar family
*/
void __cio_putchar_at( unsigned int x, unsigned int y, unsigned int c ) {
    if( ( c & 0x7f ) == '\n' ) {
        unsigned int limit;

        /*
        ** If we're in the scroll region, don't let this loop
        ** leave it.  If we're not in the scroll region, don't
        ** let this loop enter it.
        */
        if( x > scroll_max_x ) {
            limit = max_x;
        }
        else if( x >= scroll_min_x ) {
            limit = scroll_max_x;
        }
        else {
            limit = scroll_min_x - 1;
        }
        while( x <= limit ) {
            __c_putchar_at( x, y, ' ' );
            x += 1;
        }
    }
    else {
        __c_putchar_at( x, y, c );
    }
}

#ifndef SA_DEBUG
void __cio_putchar( unsigned int c ) {
    /*
    ** If we're off the bottom of the screen, scroll the window.
    */
    if( curr_y > scroll_max_y ) {
        __cio_scroll( curr_y - scroll_max_y );
        curr_y = scroll_max_y;
    }

    switch( c & BMASK8 ) {
    case '\n':
        /*
        ** Erase to the end of the line, then move to new line
        ** (actual scroll is delayed until next output appears).
        */
        while( curr_x <= scroll_max_x ) {
            __c_putchar_at( curr_x, curr_y, ' ' );
            curr_x += 1;
        }
        curr_x = scroll_min_x;
        curr_y += 1;
        break;

    case '\r':
        curr_x = scroll_min_x;
        break;

    default:
        __c_putchar_at( curr_x, curr_y, c | active_color );
        curr_x += 1;
        if( curr_x > scroll_max_x ) {
            curr_x = scroll_min_x;
            curr_y += 1;
        }
        break;
    }
    __c_setcursor();
}
#endif

/*
** The puts family
*/
void __cio_puts_at( unsigned int x, unsigned int y, const char *str ) {
    unsigned int ch;

    while( (ch = *str++) != '\0' && x <= max_x ) {
        __cio_putchar_at( x, y, ch );
        x += 1;
    }
}

#ifndef SA_DEBUG
void __cio_puts( const char *str ) {
    unsigned int ch;

    while( (ch = *str++) != '\0' ) {
        __cio_putchar( ch );
    }
}
#endif

/*
** Write a "sized" buffer (like __cio_puts(), but no NUL)
*/
void __cio_write( const char *buf, int length ) {
    for( int i = 0; i < length; ++i ) {
        __cio_putchar( buf[i] );
    }
}

void __cio_clearscroll( void ) {
    unsigned int nchars = scroll_max_x - scroll_min_x + 1;
    unsigned int l;
    unsigned int c;

    for( l = scroll_min_y; l <= scroll_max_y; l += 1 ) {
        unsigned short *to = VIDEO_ADDR( scroll_min_x, l );

        for( c = 0; c < nchars; c += 1 ) {
            *to++ = ' ' | 0x0700;
        }
    }
}

void __cio_clearscreen( void ) {
    unsigned short *to = VIDEO_ADDR( min_x, min_y );
    unsigned int    nchars = ( max_y - min_y + 1 ) * ( max_x - min_x + 1 );

    while( nchars > 0 ) {
        *to++ = ' ' | 0x0700;
        nchars -= 1;
    }
}


void __cio_scroll( unsigned int lines ) {
    unsigned short *from;
    unsigned short *to;
    int nchars = scroll_max_x - scroll_min_x + 1;
    int line, c;

    /*
    ** If # of lines is the whole scrolling region or more, just clear.
    */
    if( lines > scroll_max_y - scroll_min_y ) {
        __cio_clearscroll();
        curr_x = scroll_min_x;
        curr_y = scroll_min_y;
        __c_setcursor();
        return;
    }

    /*
    ** Must copy it line by line.
    */
    for( line = scroll_min_y; line <= scroll_max_y - lines; line += 1 ) {
        from = VIDEO_ADDR( scroll_min_x, line + lines );
        to = VIDEO_ADDR( scroll_min_x, line );
        for( c = 0; c < nchars; c += 1 ) {
            *to++ = *from++;
        }
    }

    for( ; line <= scroll_max_y; line += 1 ) {
        to = VIDEO_ADDR( scroll_min_x, line );
        for( c = 0; c < nchars; c += 1 ) {
            *to++ = ' ' | 0x0700;
        }
    }
}

static int pad( int x, int y, int extra, int padchar ) {
    while( extra > 0 ) {
        if( x != -1 || y != -1 ) {
            __cio_putchar_at( x, y, padchar );
            x += 1;
        }
        else {
            __cio_putchar( padchar );
        }
        extra -= 1;
    }
    return x;
}

static int padstr( int x, int y, char *str, int len, int width,
                   int leftadjust, int padchar ) {
    int extra;

    if( len < 0 ) {
        len = __strlen( str );
    }
    extra = width - len;
    if( extra > 0 && !leftadjust ) {
        x = pad( x, y, extra, padchar );
    }
    if( x != -1 || y != -1 ) {
        __cio_puts_at( x, y, str );
        x += len;
    }
    else {
        __cio_puts( str );
    }
    if( extra > 0 && leftadjust ) {
        x = pad( x, y, extra, padchar );
    }
    return x;
}

static void __c_do_printf( int x, int y, char **f ) {
    char *fmt = *f;
    int  *ap;
    char buf[ 12 ];
    char ch;
    char *str;
    int  leftadjust;
    int  width;
    int  len;
    int  padchar;

    /*
    ** Get characters from the format string and process them
    */

    ap = (int *)( f + 1 );

    while( (ch = *fmt++) != '\0' ) {

        /*
        ** Is it the start of a format code?
        */

        if( ch == '%' ) {

            /*
            ** Yes, get the padding and width options (if there).
            ** Alignment must come at the beginning, then fill,
            ** then width.
            */

            leftadjust = 0;
            padchar = ' ';
            width = 0;

            ch = *fmt++;

            if( ch == '-' ) {
                leftadjust = 1;
                ch = *fmt++;
            }

            if( ch == '0' ) {
                padchar = '0';
                ch = *fmt++;
            }

            while( ch >= '0' && ch <= '9' ) {
                width *= 10;
                width += ch - '0';
                ch = *fmt++;
            }

            /*
            ** What data type do we have?
            */
            switch( ch ) {

            case 'c':
                // ch = *( (int *)ap )++;
                ch = *ap++;
                buf[ 0 ] = ch;
                buf[ 1 ] = '\0';
                x = padstr( x, y, buf, 1, width, leftadjust, padchar );
                break;

            case 'd':
                // len = cvtdec( buf, *( (int *)ap )++ );
                len = __cvtdec( buf, *ap++ );
                x = padstr( x, y, buf, len, width, leftadjust, padchar );
                break;

            case 's':
                // str = *( (char **)ap )++;
                str = (char *) (*ap++);
                x = padstr( x, y, str, -1, width, leftadjust, padchar );
                break;

            case 'x':
                // len = cvthex( buf, *( (int *)ap )++ );
                len = __cvthex( buf, *ap++ );
                x = padstr( x, y, buf, len, width, leftadjust, padchar );
                break;

            case 'o':
                // len = cvtoct( buf, *( (int *)ap )++ );
                len = __cvtoct( buf, *ap++ );
                x = padstr( x, y, buf, len, width, leftadjust, padchar );
                break;

            case 'u':
                len = __cvtuns( buf, *ap++ );
                x = padstr( x, y, buf, len, width, leftadjust, padchar );
                break;

            }
        } else {

            /*
            ** No - just print it normally.
            */

            if( x != -1 || y != -1 ) {
                __cio_putchar_at( x, y, ch );
                switch( ch ) {
                case '\n':
                    y += 1;
                    /* FALL THRU */

                case '\r':
                    x = scroll_min_x;
                    break;

                default:
                    x += 1;
                }
            }
            else {
                __cio_putchar( ch );
            }
        }
    }
}

void __cio_printf_at( unsigned int x, unsigned int y, char *fmt, ... ) {
    __c_do_printf( x, y, &fmt );
}

void __cio_printf( char *fmt, ... ) {
    __c_do_printf( -1, -1, &fmt );
}

/*
** These are the "standard" IBM AT "Set 1" keycodes.
*/

static unsigned char scan_code[ 2 ][ 128 ] = {
    {   // unshifted characters
/* 00-07 */ '\377', '\033', '1',    '2',    '3',    '4',    '5',    '6',
/* 08-0f */ '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',
/* 10-17 */ 'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
/* 18-1f */ 'o',    'p',    '[',    ']',    '\n',   '\377', 'a',    's',
/* 20-27 */ 'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
/* 28-2f */ '\'',   '`',    '\377', '\\',   'z',    'x',    'c',    'v',
/* 30-37 */ 'b',    'n',    'm',    ',',    '.',    '/',    '\377', '*',
/* 38-3f */ '\377', ' ',    '\377', '\377', '\377', '\377', '\377', '\377',
/* 40-47 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '7',
/* 48-4f */ '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
/* 50-57 */ '2',    '3',    '0',    '.',    '\377', '\377', '\377', '\377',
/* 58-5f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 60-67 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 68-6f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 70-77 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 78-7f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377'
    },

    {   // shifted characters
/* 00-07 */ '\377', '\033', '!',    '@',    '#',    '$',    '%',    '^',
/* 08-0f */ '&',    '*',    '(',    ')',    '_',    '+',    '\b',   '\t',
/* 10-17 */ 'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
/* 18-1f */ 'O',    'P',    '{',    '}',    '\n',   '\377', 'A',    'S',
/* 20-27 */ 'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
/* 28-2f */ '"',    '~',    '\377', '|',    'Z',    'X',    'C',    'V',
/* 30-37 */ 'B',    'N',    'M',    '<',    '>',    '?',    '\377', '*',
/* 38-3f */ '\377', ' ',    '\377', '\377', '\377', '\377', '\377', '\377',
/* 40-47 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '7',
/* 48-4f */ '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
/* 50-57 */ '2',    '3',    '0',    '.',    '\377', '\377', '\377', '\377',
/* 58-5f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 60-67 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 68-6f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 70-77 */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 78-7f */ '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377'
    }
};

/*
** Scan code masks
*/

// 'release' bit
#define REL_BIT     0x80
#define CODE_BITS   0x7f

#define IS_PRESS(c)     (((c) & REL_BIT) == 0)
#define IS_RELEASE(c)   (((c) & REL_BIT) != 0)

/*
** Scan codes for some special characters
*/

// escape code - followed by another code byte
#define SCAN_ESC    0xe0

// shift keys:  press, release
#define L_SHIFT_DN  0x2a
#define R_SHIFT_DN  0x36
#define L_SHIFT_UP  0xaa
#define R_SHIFT_UP  0xb6

// control keys
#define L_CTRL_DN   0x1d
#define L_CTRL_UP   0x9d

/*
** I/O communication constants
*/
#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64
#define READY           0x1

/*
** Circular buffer for input characters.  Characters are inserted at
** next_space, and are removed at next_char.  Buffer is empty if
** these are equal.
*/
#define C_BUFSIZE       200

static  char    input_buffer[ C_BUFSIZE ];
static  volatile char   *next_char = input_buffer;
static  volatile char   *next_space = input_buffer;

static  volatile char *__c_increment( volatile char *pointer ) {
    if( ++pointer >= input_buffer + C_BUFSIZE ) {
        pointer = input_buffer;
    }
    return pointer;
}

static int __c_input_scan_code( int code ) {
    static  int shift = 0;
    static  int ctrl_mask = BMASK8;
    int rval = -1;

    /*
    ** Do the shift processing
    */
    code &= BMASK8;
    switch( code ) {
    case L_SHIFT_DN:
    case R_SHIFT_DN:
        shift = 1;
        break;

    case L_SHIFT_UP:
    case R_SHIFT_UP:
        shift = 0;
        break;

    case L_CTRL_DN:
        ctrl_mask = BMASK5;
        break;

    case L_CTRL_UP:
        ctrl_mask = BMASK8;
        break;

    default:
        /*
        ** Process ordinary characters only on the press
        ** (to handle autorepeat).
        ** Ignore undefined scan codes.
        */
        if( IS_PRESS(code) ) {
            code = scan_code[ shift ][ (int)code ];
            if( code != '\377' ) {
                volatile char   *next = __c_increment( next_space );

                /*
                ** Store character only if there's room
                */
                rval = code & ctrl_mask;
                if( next != next_char ) {
                    *next_space = code & ctrl_mask;
                    next_space = next;
                }
            }
        }
    }
    return( rval );
}

static void __c_keyboard_isr( int vector, int code ) {

    int data = __inb( KEYBOARD_DATA );
    int val  = __c_input_scan_code( data );

    if (data == 0xE0 && val == 0xFFFFFFFF) {
        arrow_pressed = 1;
    }
    if (data == 0xAA && val == 0xFFFFFFFF) {
        arrow_pressed = 0;
    }

    /*
	 * __cio_printf( "** CIO kbd data 0x%02x val 0x%02x\n",
     *		(unsigned int) data, (unsigned int) val );
    */

    // __vga_text_color_test(val);

    // if there is a notification function, call it
    if( val != -1 && __c_notify )
        __c_notify( val );

    __outb( PIC_PRI_CMD_PORT, PIC_EOI );
}

int __cio_getchar( void ) {
    char    c;
    int interrupts_enabled = __get_flags() & EFLAGS_IF;

    while( next_char == next_space ) {
        if( !interrupts_enabled ) {
            /*
            ** Must read the next keystroke ourselves.
            */
            while( ( __inb( KEYBOARD_STATUS ) & READY ) == 0 ) {
                ;
            }
            (void) __c_input_scan_code( __inb( KEYBOARD_DATA ) );
        }
    }

    c = *next_char & BMASK8;
    next_char = __c_increment( next_char );

    return c;
}

int __cio_gets( char *buffer, unsigned int size ) {
    char    ch;
    int count = 0;

    while( size > 1 ) {
        ch = __cio_getchar();
        if( ch == EOT ) {
            break;
        }
        *buffer++ = ch;
        count += 1;
        size -= 1;
        if( ch == '\n' ) {
            break;
        }
    }
    *buffer = '\0';
    return count;
}

int __cio_input_queue( void ) {
    int n_chars = next_space - next_char;

    if( n_chars < 0 ) {
        n_chars += C_BUFSIZE;
    }
    return n_chars;
}

/*
** Initialization routines
*/
void __cio_init( void (*fcn)(int) ) {
    /*
    ** Screen dimensions
    */
    min_x  = SCREEN_MIN_X;  
    min_y  = SCREEN_MIN_Y;
    max_x  = SCREEN_MAX_X;
    max_y  = SCREEN_MAX_Y;

    /*
    ** Scrolling region
    */
    scroll_min_x = SCREEN_MIN_X;
    scroll_min_y = SCREEN_MIN_Y;
    scroll_max_x = SCREEN_MAX_X;
    scroll_max_y = SCREEN_MAX_Y;

    /*
    ** Initial cursor location
    */
    curr_y = min_y;
    curr_x = min_x;
    __c_setcursor();

    /*
    ** Notification function (or NULL)
    */
    __c_notify = fcn;

    /*
    ** Default Text Color
    */
    _vga_text_init();

    /*
    ** Set up the interrupt handler for the keyboard
    */
    __install_isr( INT_VEC_KEYBOARD, __c_keyboard_isr );
}

#ifdef SA_DEBUG
int main() {
    __cio_printf( "%d\n", 123 );
    __cio_printf( "%d\n", -123 );
    __cio_printf( "%d\n", 0x7fffffff );
    __cio_printf( "%d\n", 0x80000001 );
    __cio_printf( "%d\n", 0x80000000 );
    __cio_printf( "x%14dy\n", 0x80000000 );
    __cio_printf( "x%-14dy\n", 0x80000000 );
    __cio_printf( "x%014dy\n", 0x80000000 );
    __cio_printf( "x%-014dy\n", 0x80000000 );
    __cio_printf( "%s\n", "xyz" );
    __cio_printf( "|%10s|\n", "xyz" );
    __cio_printf( "|%-10s|\n", "xyz" );
    __cio_printf( "%c\n", 'x' );
    __cio_printf( "|%4c|\n", 'y' );
    __cio_printf( "|%-4c|\n", 'y' );
    __cio_printf( "|%04c|\n", 'y' );
    __cio_printf( "|%-04c|\n", 'y' );
    __cio_printf( "|%3d|\n", 5 );
    __cio_printf( "|%3d|\n", 54321 );
    __cio_printf( "%x\n", 0x123abc );
    __cio_printf( "|%04x|\n", 20 );
    __cio_printf( "|%012x|\n", 0xfedcba98 );
    __cio_printf( "|%-012x|\n", 0x76543210 );
}

int curr_x, curr_y, max_x, max_y;
#endif
