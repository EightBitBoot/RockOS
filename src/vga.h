#define SP_KERNEL_SRC
#include "common.h"
#ifndef VGA_H
#define VGA_H

// Input Status #1 Register - Reading this selects Address from FlipFlop
#define INPUT_STATUS_1_PORT 0x3DA

// VGA Attribute Controller Registers
#define VGA_ATTR_ADDR_REG_RW 0x3C0 //Address and Data Writable, Address Readable
#define VGA_ATTR_ADDR_REG_DATA_RO 0x3C1 //Data Readable
// Attribute: Palette Registers are Index 0-F
#define VGA_ATTR_PALETTE_0 0x0
#define VGA_ATTR_PALETTE_1 0x1
#define VGA_ATTR_PALETTE_2 0x2
#define VGA_ATTR_PALETTE_3 0x3
#define VGA_ATTR_PALETTE_4 0x4
#define VGA_ATTR_PALETTE_5 0x5
#define VGA_ATTR_PALETTE_6 0x6
#define VGA_ATTR_PALETTE_7 0x7
#define VGA_ATTR_PALETTE_8 0x8
#define VGA_ATTR_PALETTE_9 0x9
#define VGA_ATTR_PALETTE_10 0xA
#define VGA_ATTR_PALETTE_11 0xB
#define VGA_ATTR_PALETTE_12 0xC
#define VGA_ATTR_PALETTE_13 0xD
#define VGA_ATTR_PALETTE_14 0xE
#define VGA_ATTR_PALETTE_15 0xF
// Attribute: Attribute Mode Control is Index 10
#define VGA_ATTR_MODE_CTL 0x10
// Attribute: Overscan Control is Index 11
#define VGA_ATTR_OVERSCAN_CTL 0x11
// Attribute: Color Plane Enable is Index 12
#define VGA_ATTR_COLOR_PLN_EN 0x12
// Attribute: Horizontal PEL Panning is Index 13
#define VGA_ATTR_HORIZ_PEL_PAN 0x13
// Attribute: Color Select is Index 14
#define VGA_ATTR_COLOR_SEL 0x14
// All VGA Ports needed to change modes
#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

void _reset_flipflop(void);

int _vga_attr_get_index(void);

uint8_t _vga_attr_read(unsigned int index);

void _vga_attr_write(unsigned int index, uint8_t data);

extern unsigned int vga_mode;

unsigned int _vga_get_mode(void);

void _vga_set_registers(unsigned char *regs); 

void _vga_set_mode(unsigned int graphics_text_select);

extern unsigned char g_80x25_text[];

extern unsigned char g_640x480x16[];

extern unsigned char g_320x200x256[];

void dump_regs(unsigned char *regs);

void read_regs(unsigned char *regs);

void _vga_clear_screen(void);

void draw_x(void);

void draw_test_pattern(void);

void draw_image(uint16_t im_w, uint8_t im_h, uint8_t *image_data);

extern unsigned char g_80x25_text[61];

extern unsigned char g_640x480x16[61];

extern unsigned char g_320x200x256[61];

extern unsigned char g_8x16_font[4096];

extern uint8_t g_rick[57600];

extern uint8_t g_16_color_palette[256];

extern uint8_t g_256_color_palette[768];

#endif