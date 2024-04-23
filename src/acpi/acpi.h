#ifndef ACPI_H_
#define ACPI_H_

#include "common.h"

#define _acpi_warn(message) WARNING(message);

#define _acpi_info(...) \
	do { \
		__cio_printf("[acpi][%s] ", __func__); \
		__cio_printf(__VA_ARGS__); \
		__cio_putchar('\n'); \
	} while (0);

#define ACPI_DEBUG
#ifdef ACPI_DEBUG
#define _acpi_dbg(...) _acpi_info(__VA_ARGS__);
#else
#define _acpi_dbg(...)
#endif

enum acpi_addr_space_id {
	ACPI_ADDR_SPACE_SYSTEM_MEM = 0x00,
	ACPI_ADDR_SPACE_SYSTEM_IO = 0x01,
	ACPI_ADDR_SPACE_PCI_CONFIG = 0x02,
	ACPI_ADDR_SPACE_EMBEDDED_CTRL = 0x03,
	ACPI_ADDR_SPACE_SMBUS = 0x04,
	ACPI_ADDR_SPACE_SYSTEM_CMOS = 0x05,
	ACPI_ADDR_SPACE_PCI_BAR_TARGET = 0x06,
	ACPI_ADDR_SPACE_IPMI = 0x07,
	ACPI_ADDR_SPACE_GPIO = 0x08,
	ACPI_ADDR_SPACE_GENERIC_SERIAL = 0x09,
	ACPI_ADDR_SPACE_PLATFORM_COMM = 0x0A,
	ACPI_ADDR_SPACE_PLATFORM_RUNTIME = 0x0B,

	ACPI_ADDR_SPACE_RSVD_START = 0x0C,
	ACPI_ADDR_SPACE_RSVD_END = 0x7E,

	ACPI_ADDR_SPACE_FUNC_FIXED_HW = 0x7F,
	ACPI_ADDR_SPACE_OEM_START = 0x80
};
typedef uint8_t acpi_addr_space_id;

/**
 * @struct acpi_generic_address
 * @brief ACPI Generic Address Structure
 * @sa ACPI Specification (v6.5) Section 5.2.3.2
 */
struct acpi_generic_address {
	acpi_addr_space_id addr_space_id;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t addr;
} __attribute__((packed));

// Public Types/Methods
bool_t _acpi_checksum_valid(uint8_t *ptr, uint32_t start, uint32_t length);

void _acpi_init(void);

enum _acpi_commands {
	ACPI_COMMAND_SHUTDOWN = 0,
	ACPI_COMMAND_REBOOT
};
void _acpi_command(enum _acpi_commands cmd);

#endif
