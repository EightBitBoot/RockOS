#ifndef ACPI_H_
#define ACPI_H_

#include "common.h"

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
