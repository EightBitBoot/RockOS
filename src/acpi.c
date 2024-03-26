#include "acpi.h"
#include "debug.h"
#include "cio.h"

struct acpi_data {
	struct acpi_rsdp* rsdp;

	// TODO: how to load and store AML coded tables dynamically?
};
static struct acpi_data _acpi_data = {0};

static void* _acpi_get_ebda_ptr(void) {
	uint16_t *ptr = (uint16_t *) 0x40E;

	void *ret_ptr = (void *) (*ptr << 4);
	if (ret_ptr < (void *) 0x80000 || ret_ptr > (void *) 0x9FFFF)
		WARNING("[acpi] edba ptr is outside expected range");

	return ret_ptr;
}

static struct acpi_rsdp* _acpi_find_rsdp_ptr(void) {
	char *expected_signature = ACPI_RSDP_SIGNATURE;

	// Check Extended BIOS Data Area
	void* ebda_ptr = _acpi_get_ebda_ptr();
	for (void* ptr = ebda_ptr; ptr < (void *) (ebda_ptr + 1024); ptr += 16) {
		struct acpi_rsdp *rsdp = (struct acpi_rsdp *) ptr;

		// TODO: make substring a common function
		int j;
		for (j = 0; j < sizeof(expected_signature); j++) {
			if (*(expected_signature + j) != rsdp->signature[j]) break;
		}

		// TODO: Never tested since the lab's BIOS does not provide RSDP in EBDA.
		if (j == sizeof(expected_signature)) return rsdp;
	}

	// Check 0x000E0000 to 0x000FFFFF if above fails
	for (void *ptr = (void *) 0xE0000; ptr < (void *) 0xFFFFF; ptr += 16) {
		struct acpi_rsdp *rsdp = (struct acpi_rsdp *) ptr;

		int j;
		for (j = 0; j < sizeof(expected_signature); j++) {
			if (*(expected_signature + j) != rsdp->signature[j]) break;
		}

		if (j == sizeof(expected_signature)) return rsdp;
	}

	return NULL;
}

static bool_t _acpi_check_rsdp_valid(struct acpi_rsdp* rsdp) {
	uint8_t actual_checksum;

	// ACPI 1.0 checks
	actual_checksum = 0;
	for (int i = 0; i < 20; i++) {
		actual_checksum += *(((uint8_t *) rsdp) + i);
	}

	if (actual_checksum != 0) {
		// rsdp->checksum is a value to ensure the first 20 bytes sum to 0
		WARNING("[acpi] RSDP checksum is invalid");
		return false;
	}

	// ACPI 2.0 checks
	if (rsdp->revision >= 2) {
		// >= 2 -> ACPI 2.0
		// == 0 -> ACPI 1.0
		// See ACPI Specification (v6.5) Section 5.2.5.3

		actual_checksum = 0;
		for (int i = 0; i < sizeof(struct acpi_rsdp); i++) {
			actual_checksum += *(((uint8_t *) rsdp) + i);
		}

		if (actual_checksum != 0) {
			// rsdp->extended_checksum is a value to ensure all the bytes in the table sum to 0
			WARNING("[acpi] RSDP extended checksum is invalid");
			// TODO: Just set revision to 0 since 1.0 table is fine?
			return false;
		}
	}

	return true;
}

void _acpi_init( void ) {
	__cio_printf("[acpi] initialize\n");

	// Find and ensure RSDP table is valid
	_acpi_data.rsdp = _acpi_find_rsdp_ptr();
	__cio_printf("[acpi] RSDP at 0x%x\n", _acpi_data.rsdp);

	if (!_acpi_check_rsdp_valid(_acpi_data.rsdp)) {
		_acpi_data.rsdp = NULL;
		return;
	}
	__cio_printf("[acpi] RSDP valid\n");

	// TODO: FADT, DSDT

	// TODO: ACPI enable (section 16.3.1)

	// TODO: syscalls for changing sleep state (shutdown/reboot/reset/sleep/etc.)

	__cio_printf("[acpi] initialize complete\n");
}
