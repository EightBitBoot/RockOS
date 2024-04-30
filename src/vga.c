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

/**
 * Get whether VGA is in Text or Graphics Mode
 * Returns 0 if Text Mode
 * Returns 1 if Graphics Mode
*/
unsigned int _vga_get_graphics_text_select() {
    uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    return attr_mode_ctl & 0b00000001;
}

/**
 * Set whether VGA is in Text or Graphics Mode
 * 0 sets Text Mode
 * 1 sets Graphics Mode
*/
void _vga_set_graphics_text_select(unsigned int graphics_text_select) {
    uint8_t attr_mode_ctl = _vga_attr_read(VGA_ATTR_MODE_CTL);
    if (graphics_text_select) {
        _vga_attr_write(VGA_ATTR_MODE_CTL, attr_mode_ctl & 0b11111110);
    } else {
        _vga_attr_write(VGA_ATTR_MODE_CTL, attr_mode_ctl | 0b00000001);   
    }
}