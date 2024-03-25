#ifndef ACPI_H_
#define ACPI_H_

#include "common.h"

#define ACPI_RSDP_SIGNATURE "RSD PTR "
struct acpi_rsdp {
	// ACPI 1.0
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;

	// ACPI 2.0 (revision == 2)
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
};

void _acpi_init(void);

#endif
