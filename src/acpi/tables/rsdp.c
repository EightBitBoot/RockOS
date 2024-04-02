#include <acpi/acpi.h>
#include <acpi/tables/rsdp.h>
#include "debug.h"

static void* _acpi_get_ebda_ptr(void) {
	uint16_t *ptr = (uint16_t *) 0x40E;

	void *ret_ptr = (void *) (*ptr << 4);
	if (ret_ptr < (void *) 0x80000 || ret_ptr > (void *) 0x9FFFF)
		WARNING("[acpi] edba ptr is outside expected range");

	return ret_ptr;
}

static bool_t _acpi_validate_rsdp(struct acpi_rsdp* rsdp) {
	// Verify valid signature
	char *expected_signature = ACPI_RSDP_SIGNATURE;

	int j;
	for (j = 0; j < sizeof(expected_signature); j++) {
		if (expected_signature[j] != rsdp->signature[j]) break;
	}

	if (j != sizeof(expected_signature)) {
		// No output here, since this is called in a loop
		return false;
	}

	// ACPI 1.0 checks
	if (!_acpi_checksum_valid((uint8_t *) rsdp, 0, 20)) {
		// rsdp->checksum is a value to ensure the first 20 bytes sum to 0
		WARNING("[acpi] RSDP checksum is invalid, ignoring");
		return false;
	}

	// ACPI 2.0 checks
	if (rsdp->revision >= 2) {
		// >= 2 -> ACPI 2.0
		// == 0 -> ACPI 1.0
		// See ACPI Specification (v6.5) Section 5.2.5.3
		if (!_acpi_checksum_valid((uint8_t *) rsdp, 0, sizeof(struct acpi_rsdp))) {
			// rsdp->extended_checksum is a value to ensure all the bytes in the table sum to 0
			WARNING("[acpi] RSDP extended checksum is invalid, ignoring");
			// TODO: Just set revision to 0 since 1.0 table is fine?
			return false;
		}
	}

	// Invalid revision check
	if (rsdp->revision == 1) {
		// Not ACPI 1.0, not APCI 2.0, invalid
		WARNING("[acpi] RSDP claims support for imaginary revision, ignoring");
		return false;
	}

	return true;
}

struct acpi_rsdp* _acpi_get_rsdp_ptr(void) {
	// TODO: Find RSDP from UEFI

	// Check Extended BIOS Data Area
	void* ebda_ptr = _acpi_get_ebda_ptr();
	for (void* ptr = ebda_ptr; ptr < (void *) (ebda_ptr + 1024); ptr += 16) {
		struct acpi_rsdp *rsdp = (struct acpi_rsdp *) ptr;

		if (_acpi_validate_rsdp(rsdp))
			return rsdp;
	}

	// Check 0x000E0000 to 0x000FFFFF if above fails
	for (void *ptr = (void *) 0xE0000; ptr < (void *) 0xFFFFF; ptr += 16) {
		struct acpi_rsdp *rsdp = (struct acpi_rsdp *) ptr;

		if (_acpi_validate_rsdp(rsdp))
			return rsdp;
	}

	return NULL;
}
