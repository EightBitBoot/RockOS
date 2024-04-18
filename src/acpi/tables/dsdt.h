#include <acpi/acpi.h>
#include <acpi/tables/sdt.h>

// DSDT and SSDTs have the same structure
#define ACPI_DSDT_SIGNATURE "DSDT"
#define ACPI_SSDT_SIGNATURE "SSDT"
/**
 * @struct acpi_dsdt
 * @brief Differentiated System Description Table
 * @sa ACPI Specification (v6.5) Section 5.2.11.1
 */
struct acpi_dsdt {
	struct acpi_sdt_header header;

	uint8_t definition_block[1];
};
