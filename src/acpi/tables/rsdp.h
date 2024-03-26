#ifndef ACPI_RSDP_H_
#define ACPI_RSDP_H_

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

struct acpi_rsdp* _acpi_get_rsdp_ptr(void);

#endif
