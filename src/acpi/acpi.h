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

/**
 * @struct acpi_generic_address
 * @brief ACPI Generic Address Structure
 * @sa ACPI Specification (v6.5) Section 5.2.3.2
 */
struct acpi_generic_address {
	uint8_t addr_space_id;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t addr;
} __attribute__((packed));

bool_t _acpi_checksum_valid(uint8_t *ptr, uint32_t start, uint32_t length);

void _acpi_init(void);

#endif
