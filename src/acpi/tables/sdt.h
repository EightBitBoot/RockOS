#ifndef ACPI_SDT_H_
#define ACPI_SDT_H_

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

#endif
