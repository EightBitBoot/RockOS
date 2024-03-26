#ifndef ACPI_H_
#define ACPI_H_

#include "common.h"

#define ACPI_RSDP_SIGNATURE "RSD PTR "
/**
 * @struct acpi_rsdp
 * @brief ACPI Root System Description Pointer
 * @sa ACPI Specification (v6.5) Section 5.2.5.3
 */
struct acpi_rsdp {
	// ACPI 1.0
	char signature[8]; // Should be equal to ACPI_RSDP_SIGNATURE
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;

	// ACPI 2.0 (revision >= 2)
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__((packed));

/**
 * @struct acpi_sdt_header
 * @brief ACPI System Description Table Header
 * @sa ACPI Specification (v6.5) Section 5.2.6
 */
struct acpi_sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;

	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;

	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

void _acpi_init(void);

#endif
