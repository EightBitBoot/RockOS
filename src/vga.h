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

void _reset_flipflop(void);

int _vga_attr_get_index(void);

uint8_t _vga_attr_read(unsigned int index);

void _vga_attr_write(unsigned int index, uint8_t data);

unsigned int _vga_get_graphics_text_select(void);

void _vga_set_graphics_text_select(unsigned int graphics_text_select);

#endif