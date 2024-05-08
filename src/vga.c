#include "vga.h"
#include "lib.h"
#include "ulib.h"
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

void _reset_flipflop() {
    // Read Input Status #1 to set FlipFlop to Index
    __inb(INPUT_STATUS_1_PORT); // Read port 3DA
}

int _vga_attr_get_index() {
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
    sprint(buf, "var oi: 0x%x, mi: 0x%x, r: 0x%x\r\n", orig_index, mod_index, retval);
    _sio_puts(buf);
    return retval;
}

void _vga_attr_write(unsigned int index, uint8_t data) {
    // Read value of Address/Data registers
    // _reset_flipflop();
    // int orig_addr_reg = __inb(VGA_ATTR_ADDR_REG_RW);
    // int orig_addr_dat = _vga_attr_read(index);
    _reset_flipflop();
    int orig_index = __inb(VGA_ATTR_ADDR_REG_RW);
    char buf[256];
    sprint(buf, "vaw i: 0x%x, d: 0x%x, o: 0x%x\r\n", index, data, orig_index);
    _sio_puts(buf);
    int mod_index = (orig_index & 0b11100000) | (index & 0b00011111);
    sprint(buf, "vaw: mi: 0x%x\r\n", mod_index);
    _sio_puts(buf);
    _reset_flipflop();
    __outb(VGA_ATTR_ADDR_REG_RW, mod_index);
    __outb(VGA_ATTR_ADDR_REG_RW, data);
    _reset_flipflop();
}

unsigned int vga_mode = 0;

/**
 * Get whether VGA is in Text or Graphics Mode
 * Returns 0 if Text Mode
 * Returns 1 if Graphics Mode
*/
unsigned int _vga_get_mode() {
    // uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    // return attr_mode_ctl & 0b00000001;
    return vga_mode;
}

void _vga_set_registers(unsigned char *regs) {
unsigned int i;

/* write MISCELLANEOUS reg */
	__outb(VGA_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		__outb(VGA_SEQ_INDEX, i);
		__outb(VGA_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	__outb(VGA_CRTC_INDEX, 0x03);
	__outb(VGA_CRTC_DATA, __inb(VGA_CRTC_DATA) | 0x80);
	__outb(VGA_CRTC_INDEX, 0x11);
	__outb(VGA_CRTC_DATA, __inb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		__outb(VGA_CRTC_INDEX, i);
		__outb(VGA_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		__outb(VGA_GC_INDEX, i);
		__outb(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)__inb(VGA_INSTAT_READ);
		__outb(VGA_AC_INDEX, i);
		__outb(VGA_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)__inb(VGA_INSTAT_READ);
	__outb(VGA_AC_INDEX, 0x20);
}

static void set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	__outb(VGA_GC_INDEX, 4);
	__outb(VGA_GC_DATA, p);
/* set write plane */
	__outb(VGA_SEQ_INDEX, 2);
	__outb(VGA_SEQ_DATA, pmask);
}

static unsigned get_fb_seg(void)
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
        _sio_puts("1");
		seg = 0xA0000;
		break;
	case 2:
        _sio_puts("2");
		seg = 0xB0000;
		break;
	case 3:
        _sio_puts("3");
		seg = 0xB8000;
		break;
	}
	return seg;
}

static void vpokeb(unsigned off, unsigned val)
{
	unsigned *pokeb = (unsigned*)(get_fb_seg()+off);
    *pokeb = val;
}
/*****************************************************************************
*****************************************************************************/
static unsigned vpeekb(unsigned off)
{
    unsigned *peekb = (unsigned*)(get_fb_seg()+off);
	return (*peekb);
}

static void write_pixel_noop(unsigned x, unsigned y, unsigned c) {
    return;
}

static void (*g_write_pixel)(unsigned, unsigned, unsigned) = write_pixel_noop;
static unsigned int g_wd = 0;
static unsigned int g_ht = 0;
static unsigned int g_c = 0;

static void write_pixel4p(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes, off, mask, p, pmask;

	wd_in_bytes = g_wd / 8;
	off = wd_in_bytes * y + x / 8;
	x = (x & 7) * 1;
	mask = 0x80 >> x;
	pmask = 1;
	for(p = 0; p < 4; p++)
	{
		set_plane(p);
		if(pmask & c)
			vpokeb(off, vpeekb(off) | mask);
		else
			vpokeb(off, vpeekb(off) & ~mask);
		pmask <<= 1;
	}
}

static void write_pixel8(unsigned x, unsigned y, unsigned c)
{
	unsigned wd_in_bytes;
	unsigned off;

	wd_in_bytes = g_wd;
	off = wd_in_bytes * y + x;
	vpokeb(off, c);
}

void _vga_clear_screen(void) {
    unsigned x, y;

/* clear screen */
	for (y = 0; y < 4; y++) {
		set_plane(y);
		__memclr((unsigned *) 0xA0000, 64000);
	}
}

void draw_x(void) {
	unsigned x, y;

/* draw 2-color X */
	for(y = 0; y < g_ht; y++)
	{
		g_write_pixel((g_wd - g_ht) / 2 + y, y, 1);
		g_write_pixel((g_ht + g_wd) / 2 - y, y, 2);
	}
}

void draw_test_pattern(void) {
    unsigned x, y;
    unsigned w_frac, h_frac;
	if (vga_mode == 1) {
		w_frac = g_wd/16;
		h_frac = 1;
	} else if (vga_mode == 2) {
		w_frac = g_wd/16;
		h_frac = g_ht/16;
	}
	for (y = 0; y < g_ht; y++) {
        for (x = 0; x < g_wd; x++) {
            g_write_pixel(x, y, (x/w_frac)+(16*(y/h_frac)));
        }
    }
}

void draw_rick(void) {
	unsigned x, y;
	for (y = 0; y < 180; y++) {
		for (x = 0; x < 320; x++) {
			g_write_pixel(x, y, g_rick[y*319+x]);
		}
	}
}

static void write_font(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;
	// Save Registers, as well as GC 4 and SEQ 2, since set_plane modifies them too
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
	set_plane(2);
	// Write Font 0
	for(i = 0; i < 256; i++)
	{
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

static void write_color_palette(uint8_t *palette, unsigned num_colors) {
	unsigned i,r,g,b;
	char buf[32];

	sprint(buf, "\r\nPEL: %x", __inb(0x3C6));
	_sio_puts(buf);
	__outb(0x3C6,0xFF);
	__outb(0x3C8,0);
	for (i = 0; i < 256; i++) {
		r = __inb(0x3C9)<<2;
		g = __inb(0x3C9)<<2;
		b = __inb(0x3C9)<<2;
		sprint(buf, "\r\ni:%d,rgb:%d,%d,%d",i,r,g,b);
		_sio_puts(buf);
	}
	__outb(0x3C8,0);
	for (i = 0; i < num_colors*3; i++) {
		__outb(0x3C9,palette[i]);
	}
}

static void restore_font(void) {
	write_font(g_8x16_font, 16);
    // unsigned x, y, i;
	// char buf[24];
	// for (i = 0; i < 4; i++) {
	// 	sprint(buf, "\r\nPlane %d... ", i);
	// 	_sio_puts(buf);
	// 	set_plane(i);
	// 	__memcpy((unsigned*) 0xA0000+(64000*i), font_backup, 64000);
	// 	_sio_puts("Restored!");
	// }
    // for (y = 0; y < g_ht; y++) {
    //     for (x = 0; x < g_wd; x++) {
	// 		write_pixel4p(x, y, font_backup[x*480+y]);
    //     }
    // }
}

/**
 * Set whether VGA is in Text or Graphics Mode
 * 0 sets Text Mode
 * 1 sets Graphics Mode
*/
void _vga_set_mode(unsigned int target_mode) {
    // uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    switch(target_mode) {
        case 0:
			_sio_puts("\r\nRestore Font\r\n");
			_vga_set_registers(g_640x480x16);
			restore_font();
            vga_mode = 0;
            _sio_puts("\r\nEnter Text Mode\r\n");    
            _vga_set_registers(g_80x25_text);
            __cio_clearscreen();
            break;
        case 1:
            vga_mode = 1;
			g_write_pixel = write_pixel4p;
			g_wd = 640;
			g_ht = 480;
			g_c = 16;
            _sio_puts("\r\nEnter 16-color 640x480 Graphics Mode\r\n");
            _vga_set_registers(g_640x480x16);
            break;
		case 2:
			vga_mode = 2;
			g_write_pixel = write_pixel8;
			g_wd = 320;
			g_ht = 200;
			g_c = 256;
			write_color_palette(g_256_color_palette, 256);
			_sio_puts("\r\nEnter 256-color 320x200 Graphics Mode\r\n");
			_vga_set_registers(g_320x200x256);
			break;
    }
}

static void dump(unsigned char *regs, unsigned count)
{
	unsigned i;

	i = 0;
	__cio_printf("\t");
	for(; count != 0; count--)
	{
		__cio_printf("0x%02x,", *regs);
		i++;
		if(i >= 8)
		{
			i = 0;
			__cio_printf("\n\t");
		}
		else
			__cio_printf(" ");
		regs++;
	}
	__cio_printf("\n");
}

void dump_regs(unsigned char *regs)
{
	__cio_printf("unsigned char g_mode[] =\n");
	__cio_printf("{\n");
/* dump MISCELLANEOUS reg */
	__cio_printf("/* MISC */\n");
	__cio_printf("\t0x%02x,\n", *regs);
	regs++;
/* dump SEQUENCER regs */
	__cio_printf("/* SEQ */\n");
	dump(regs, VGA_NUM_SEQ_REGS);
	regs += VGA_NUM_SEQ_REGS;
/* dump CRTC regs */
	__cio_printf("/* CRTC */\n");
	dump(regs, VGA_NUM_CRTC_REGS);
	regs += VGA_NUM_CRTC_REGS;
/* dump GRAPHICS CONTROLLER regs */
	__cio_printf("/* GC */\n");
	dump(regs, VGA_NUM_GC_REGS);
	regs += VGA_NUM_GC_REGS;
/* dump ATTRIBUTE CONTROLLER regs */
	__cio_printf("/* AC */\n");
	dump(regs, VGA_NUM_AC_REGS);
	regs += VGA_NUM_AC_REGS;
	__cio_printf("};\n");
}

void read_regs(unsigned char *regs) {
	unsigned i;

/* read MISCELLANEOUS reg */
	*regs = __inb(VGA_MISC_READ);
	regs++;
/* read SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		__outb(VGA_SEQ_INDEX, i);
		*regs = __inb(VGA_SEQ_DATA);
		regs++;
	}
/* read CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		__outb(VGA_CRTC_INDEX, i);
		*regs = __inb(VGA_CRTC_DATA);
		regs++;
	}
/* read GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		__outb(VGA_GC_INDEX, i);
		*regs = __inb(VGA_GC_DATA);
		regs++;
	}
/* read ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)__inb(VGA_INSTAT_READ);
		__outb(VGA_AC_INDEX, i);
		*regs = __inb(VGA_AC_READ);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)__inb(VGA_INSTAT_READ);
	__outb(VGA_AC_INDEX, 0x20);
}