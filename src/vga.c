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

static void *g_write_pixel = NULL;
static unsigned int g_wd = 640;
static unsigned int g_ht = 480;
static unsigned int g_c = 16;

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

void _vga_clear_screen(void) {
    unsigned x, y;

/* clear screen */
	for(y = 0; y < g_ht; y++)
		for(x = 0; x < g_wd; x++)
			write_pixel4p(x, y, 0);
}

void draw_x(void) {
	unsigned x, y;

/* draw 2-color X */
	for(y = 0; y < g_ht; y++)
	{
		write_pixel4p((g_wd - g_ht) / 2 + y, y, 1);
		write_pixel4p((g_ht + g_wd) / 2 - y, y, 2);
	}
}

void draw_gradient(void) {
    unsigned x, y;
    unsigned frac = g_wd/g_c;
    for (y = 0; y < g_ht; y++) {
        for (x = 0; x < g_wd; x++) {
            write_pixel4p(x, y, x/frac);
        }
    }
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
            vga_mode = 0;
            _sio_puts("\r\nEnter Text Mode\r\n");    
            _vga_set_registers(g_80x25_text);
            __cio_clearscreen();
            break;
        case 1:
            vga_mode = 1;
            _sio_puts("\r\nEnter 16-color 640x480 Graphics Mode\r\n");
            _vga_set_registers(g_640x480x16);
            // draw_gradient();
            // draw_x();
            // _vga_attr_write(VGA_ATTR_MODE_CTL, attr_mode_ctl | 0b00000001);
            break;
    }
}

unsigned char g_80x25_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x07, 0x80,
	0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x04, 0x00, 0x0F, 0x08, 0x00
};

unsigned char g_640x480x16[] =
{
/* MISC */
	0xE3,
/* SEQ */
	0x03, 0x01, 0x08, 0x00, 0x06,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x01, 0x00, 0x0F, 0x00, 0x00
};

unsigned char g_320x200x256[] =
{
/* MISC */
	0x63,
/* SEQ */
	0x03, 0x01, 0x0F, 0x00, 0x0E,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, 0x00, 0x0F, 0x00,	0x00
};

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
/*****************************************************************************
*****************************************************************************/
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