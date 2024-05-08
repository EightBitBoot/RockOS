#include "vga.h"
#include "libc/lib.h"
#include "usr/ulib.h"
#define SP_KERNEL_SRC
#include "common.h"
#include "sio.h"

// https://mirrors.apple2.org.za/ftp.apple.asimov.net/documentation/hardware/video/Second%20Sight%20VGA%20Registers.pdf
// http://vgamuseum.info/images/doc/chips/82c453.pdf
// https://wiki.osdev.org/VGA_Hardware
// http://www.osdever.net/FreeVGA/vga/vgareg.htm
// http://www.osdever.net/FreeVGA/vga/attrreg.htm
// http://www.osdever.net/FreeVGA/vga/vga.htm
// http://www.mcamafia.de/pdf/ibm_vgaxga_trm2.pdf
// https://fd.lod.bz/rbil/ports/video/p03c003c1.html

static unsigned int _vga_mode = 0;

static void _reset_flipflop() {
    // Read Input Status #1 to set FlipFlop to Index
    __inb(INPUT_STATUS_1_PORT); // Read port 3DA
}

static int _vga_attr_get_index() {
    _reset_flipflop();
    return __inb(VGA_ATTR_ADDR_REG_RW);
    _reset_flipflop();
}

uint8_t _vga_attr_read(unsigned int index) {
    _reset_flipflop();
    char buf[256];
    int orig_index = __inb(VGA_ATTR_ADDR_REG_RW);
    int mod_index = (orig_index & 0b11100000) | (index & 0b00011111);
    __outb(VGA_ATTR_ADDR_REG_RW, mod_index);
    // Save current value of Address/Data Register
    uint8_t retval = (uint8_t) __inb(VGA_ATTR_ADDR_REG_DATA_RO);
    _reset_flipflop();
    return retval;
}

void _vga_attr_write(unsigned int index, uint8_t data) {
    // Read value of Address/Data registers
    _reset_flipflop();
    int orig_index = __inb(VGA_ATTR_ADDR_REG_RW);
    int mod_index = (orig_index & 0b11100000) | (index & 0b00011111);
    _reset_flipflop();
    __outb(VGA_ATTR_ADDR_REG_RW, mod_index);
    __outb(VGA_ATTR_ADDR_REG_RW, data);
    _reset_flipflop();
}

/**
 * Get whether VGA is in Text or Graphics Mode
 * Returns 0 if Text Mode
 * Returns 1 if 16-color 640x480 Graphics Mode
 * Returns 2 if 256-color 320x200 Graphics Mode
*/
unsigned int __vga_get_mode() {
    return _vga_mode;
}

static void _vga_set_registers(unsigned char *regs) {
	unsigned int i;

	// Write Miscellaneous Register */
	__outb(VGA_MISC_WRITE, *regs);
	regs++;
	// Write Sequencer Registers */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++) {
		__outb(VGA_SEQ_INDEX, i);
		__outb(VGA_SEQ_DATA, *regs);
		regs++;
	}
	// Unlock CRTC Registers
	__outb(VGA_CRTC_INDEX, 0x03);
	__outb(VGA_CRTC_DATA, __inb(VGA_CRTC_DATA) | 0x80);
	__outb(VGA_CRTC_INDEX, 0x11);
	__outb(VGA_CRTC_DATA, __inb(VGA_CRTC_DATA) & ~0x80);
	// Make sure they remain unlocked
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
	// Write CRTC Registers
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++) {
		__outb(VGA_CRTC_INDEX, i);
		__outb(VGA_CRTC_DATA, *regs);
		regs++;
	}
	// Write Graphics Controller Registers */
	for(i = 0; i < VGA_NUM_GC_REGS; i++) {
		__outb(VGA_GC_INDEX, i);
		__outb(VGA_GC_DATA, *regs);
		regs++;
	}
	// Write Attribute Controller Registers */
	for(i = 0; i < VGA_NUM_AC_REGS; i++) {
		(void)__inb(VGA_INSTAT_READ);
		__outb(VGA_AC_INDEX, i);
		__outb(VGA_AC_WRITE, *regs);
		regs++;
	}
	// Lock 16-color Palette and unblank display */
	(void)__inb(VGA_INSTAT_READ);
	__outb(VGA_AC_INDEX, 0x20);
}

static void _set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
	// Set Read Plane
	__outb(VGA_GC_INDEX, 4);
	__outb(VGA_GC_DATA, p);
	// Set Write Plane
	__outb(VGA_SEQ_INDEX, 2);
	__outb(VGA_SEQ_DATA, pmask);
}

static unsigned _get_fb_seg(void)
{
	unsigned seg;

	__outb(VGA_GC_INDEX, 6);
	seg = __inb(VGA_GC_DATA);
	seg >>= 2;
	seg &= 3;
	switch(seg)
	{
	case 0:
	case 1:
		seg = 0xA0000;
		break;
	case 2:
		seg = 0xB0000;
		break;
	case 3:
		seg = 0xB8000;
		break;
	}
	return seg;
}

static void _vpokeb(unsigned off, unsigned val)
{
	unsigned *pokeb = (unsigned*)(_get_fb_seg()+off);
    *pokeb = val;
}

static unsigned _vpeekb(unsigned off)
{
    unsigned *peekb = (unsigned*)(_get_fb_seg()+off);
	return (*peekb);
}

static void _write_pixel_noop(unsigned x, unsigned y, unsigned c) {
    return;
}

void (*__vga_write_pixel)(unsigned, unsigned, unsigned) = _write_pixel_noop;
static unsigned int _current_width = 0;
static unsigned int _current_height = 0;
static unsigned int _current_colors = 0;

static void _write_pixel4p(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes, off, mask, p, pmask;

	wd_in_bytes = _current_width / 8;
	off = wd_in_bytes * y + x / 8;
	x = (x & 7) * 1;
	mask = 0x80 >> x;
	pmask = 1;
	for(p = 0; p < 4; p++) {
		_set_plane(p);
		if(pmask & c) {
			_vpokeb(off, _vpeekb(off) | mask);
		} else {
			_vpokeb(off, _vpeekb(off) & ~mask);
		}
		pmask <<= 1;
	}
}

static void _write_pixel8(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes;
	unsigned off;

	wd_in_bytes = _current_width;
	off = wd_in_bytes * y + x;
	_vpokeb(off, c);
}

void __vga_clear_screen(void) {
    unsigned x, y;
	unsigned mode = _vga_mode;

	// Clear Screen
	for (y = 0; y < 4; y++) {
		_set_plane(y);
		__memclr((unsigned *) 0xA0000, 64000);
	}

	__vga_set_mode(mode);
}

void _draw_x(void) {
	unsigned x, y;

	for(y = 0; y < _current_height; y++) {
		__vga_write_pixel((_current_width - _current_height) / 2 + y, y, 1);
		__vga_write_pixel((_current_height + _current_width) / 2 - y, y, 2);
	}
}

void __vga_draw_test_pattern(void) {
    unsigned x, y;
    unsigned w_frac, h_frac;
	if (_vga_mode == 1) {
		w_frac = _current_width/16;
		h_frac = 1;
	} else if (_vga_mode == 2) {
		w_frac = _current_width/16;
		h_frac = _current_height/16;
	}
	for (y = 0; y < _current_height; y++) {
        for (x = 0; x < _current_width; x++) {
			if (_vga_mode == 1) { // Draw a Test Pattern consisting of vertical color bars
				__vga_write_pixel(x, y, x/w_frac);
			} else if (_vga_mode == 2) { // Draw a Test Pattern depicting the 256-color Palette
				__vga_write_pixel(x, y, (x/w_frac)+(15*(y/h_frac)));
			}
        }
    }
}

void __vga_draw_image(uint16_t im_w, uint8_t im_h, uint8_t off_x, uint8_t off_y, uint8_t *image_data) {
	unsigned x, y;
	for (y = 0; y < im_h; y++) {
		for (x = 0; x < im_w; x++) {
			__vga_write_pixel(x+off_x, y+off_y, image_data[y*im_w+x]);
		}
	}
}

static void _write_font(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;
	// Save Registers, as well as GC 4 and SEQ 2, since _set_plane modifies them too
	__outb(VGA_SEQ_INDEX, 2);
	seq2 = __inb(VGA_SEQ_DATA);

	__outb(VGA_SEQ_INDEX, 4);
	seq4 = __inb(VGA_SEQ_DATA);
	// Turn off even-odd addressing (move to flat addressing), assume chain-4 addressing is off
	__outb(VGA_SEQ_DATA, seq4 | 0x04);

	__outb(VGA_GC_INDEX, 4);
	gc4 = __inb(VGA_GC_DATA);

	__outb(VGA_GC_INDEX, 5);
	gc5 = __inb(VGA_GC_DATA);
	// Turn off odd/even addressing */
	__outb(VGA_GC_DATA, gc5 & ~0x10);

	__outb(VGA_GC_INDEX, 6);
	gc6 = __inb(VGA_GC_DATA);
	__outb(VGA_GC_DATA, gc6 & ~0x02);
	// Write font to plane P4
	_set_plane(2);
	// Write Font 0
	for(i = 0; i < 256; i++) {
		__memcpy(0xA0000+(16384u * 0 + i * 32), buf, font_height);
		buf += font_height;
	}
	// Restore Registers
	__outb(VGA_SEQ_INDEX, 2);
	__outb(VGA_SEQ_DATA, seq2);
	__outb(VGA_SEQ_INDEX, 4);
	__outb(VGA_SEQ_DATA, seq4);
	__outb(VGA_GC_INDEX, 4);
	__outb(VGA_GC_DATA, gc4);
	__outb(VGA_GC_INDEX, 5);
	__outb(VGA_GC_DATA, gc5);
	__outb(VGA_GC_INDEX, 6);
	__outb(VGA_GC_DATA, gc6);
}

static void _write_color_palette(uint8_t *palette, unsigned num_colors) {
	unsigned i,r,g,b;
	char buf[32];

	__outb(VGA_DAC_WRITE_INDEX, 0);
	for (i = 0; i < num_colors*3; i++) {
		__outb(VGA_DAC_DATA ,palette[i]);
	}
}

static void _restore_font(void) {
	_write_font(_vga_font_default, 16);
}

/**
 * Set whether VGA is in Text or Graphics Mode
 * 0 sets Text Mode
 * 1 sets 16-color 640x480 Graphics Mode
 * 2 sets 256-color 320x200 Graphics Mode
*/
void __vga_set_mode(unsigned int target_mode) {
    switch(target_mode) {
        case 0:
			_sio_puts("\r\nRestore Font\r\n");
			_vga_set_registers(_vga_mode_640x480x16_graphics); // I don't know why I need to change to 16-color mode to get the font to restore correctly - judging by the artifacts, it might be something to do with planar?
			_restore_font();
			_sio_puts("\r\nRestore Color Palette\r\n");
			_write_color_palette(_vga_palette_16, 64);
            _vga_mode = 0;
			__vga_write_pixel = _write_pixel_noop;
			_current_width = 0;
			_current_height = 0;
			_current_colors = 16;
            _sio_puts("\r\nEnter Text Mode\r\n");    
            _vga_set_registers(_vga_mode_80x25_text);
            __cio_clearscreen();
            break;
        case 1:
            _vga_mode = 1;
			__vga_write_pixel = _write_pixel4p;
			_current_width = 640;
			_current_height = 480;
			_current_colors = 16;
            _sio_puts("\r\nEnter 16-color 640x480 Graphics Mode\r\n");
            _vga_set_registers(_vga_mode_640x480x16_graphics);
            break;
		case 2:
			_vga_mode = 2;
			__vga_write_pixel = _write_pixel8;
			_current_width = 320;
			_current_height = 200;
			_current_colors = 256;
			_write_color_palette(_vga_palette_256, 256);
			_sio_puts("\r\nEnter 256-color 320x200 Graphics Mode\r\n");
			_vga_set_registers(_vga_mode_320x200x256_graphics);
			break;
    }
}

static void _reg_dump(unsigned char *regs, unsigned count)
{
	unsigned i;

	i = 0;
	__cio_printf("\t");
	for(; count != 0; count--) {
		__cio_printf("0x%02x,", *regs);
		i++;
		if(i >= 8) {
			i = 0;
			__cio_printf("\n\t");
		} else {
			__cio_printf(" ");
		}
		regs++;
	}
	__cio_printf("\n");
}

void _dump_regs(unsigned char *regs)
{
	__cio_printf("unsigned char g_mode[] =\n");
	__cio_printf("{\n");
	// Dump Miscellaneous Register
	__cio_printf("/* MISC */\n");
	__cio_printf("\t0x%02x,\n", *regs);
	regs++;
	// Dump Sequencer Registers
	__cio_printf("/* SEQ */\n");
	_reg_dump(regs, VGA_NUM_SEQ_REGS);
	regs += VGA_NUM_SEQ_REGS;
	// Dump CRTC Register
	__cio_printf("/* CRTC */\n");
	_reg_dump(regs, VGA_NUM_CRTC_REGS);
	regs += VGA_NUM_CRTC_REGS;
	// Dump Graphics Controller Registers
	__cio_printf("/* GC */\n");
	_reg_dump(regs, VGA_NUM_GC_REGS);
	regs += VGA_NUM_GC_REGS;
	// Dump Attribute Controller Registers
	__cio_printf("/* AC */\n");
	_reg_dump(regs, VGA_NUM_AC_REGS);
	regs += VGA_NUM_AC_REGS;
	__cio_printf("};\n");
}

void _read_regs(unsigned char *regs) {
	unsigned i;

	// Read Miscellaneous Register
	*regs = __inb(VGA_MISC_READ);
	regs++;
	// Read Sequencer Registers
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++) {
		__outb(VGA_SEQ_INDEX, i);
		*regs = __inb(VGA_SEQ_DATA);
		regs++;
	}
	// Read CRTC Registers
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++) {
		__outb(VGA_CRTC_INDEX, i);
		*regs = __inb(VGA_CRTC_DATA);
		regs++;
	}
	// Read Graphics Controller Registers
	for(i = 0; i < VGA_NUM_GC_REGS; i++) {
		__outb(VGA_GC_INDEX, i);
		*regs = __inb(VGA_GC_DATA);
		regs++;
	}
	// Read Attribute Controller Registers
	for(i = 0; i < VGA_NUM_AC_REGS; i++) {
		(void)__inb(VGA_INSTAT_READ);
		__outb(VGA_AC_INDEX, i);
		*regs = __inb(VGA_AC_READ);
		regs++;
	}
	// Lock 16-color Palette and unblank display
	(void)__inb(VGA_INSTAT_READ);
	__outb(VGA_AC_INDEX, 0x20);
}