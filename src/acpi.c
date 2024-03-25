#include "acpi.h"
#include "debug.h"
#include "cio.h"

void* _acpi_get_ebda_ptr(void) {
	uint16_t *ptr = (uint16_t *) 0x40E;

	void *ret_ptr = (void *) (*ptr << 4);
	if (ret_ptr < (void *) 0x80000 || ret_ptr > (void *) 0x9FFFF)
		WARNING("[acpi] edba ptr is outside expected range");

	return ret_ptr;
}

struct acpi_rsdp* _acpi_get_rsdp_ptr(void) {
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

		// TODO: Never tested since the lab's BIOS does not provide RSDP in EBDA.
		if (j == sizeof(expected_signature)) return rsdp;
	}

	return NULL;
}

void _acpi_init( void ) {
	__cio_printf("[acpi] initialize\n");

	struct acpi_rsdp* rsdp_ptr = _acpi_get_rsdp_ptr();
	__cio_printf("[acpi] RSDP at 0x%x\n", rsdp_ptr);
	// TODO: check checksum, store this somewhere?

	__cio_printf("[acpi] initialize complete\n");
}
