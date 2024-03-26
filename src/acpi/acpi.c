#include <acpi/acpi.h>
#include <acpi/tables/rsdp.h>
#include "cio.h"

struct acpi_data {
	struct acpi_rsdp* rsdp;

	// TODO: how to load and store AML coded tables dynamically?
};
static struct acpi_data _acpi_data = {0};

void _acpi_init(void) {
	__cio_printf("[acpi] initialize\n");

	// Find and ensure RSDP table is valid
	_acpi_data.rsdp = _acpi_get_rsdp_ptr();
	if (_acpi_data.rsdp) {
		__cio_printf("[acpi] RSDP at 0x%x\n", _acpi_data.rsdp);
	} else {
		__cio_printf("[acpi] System ACPI compatibility not detected\n");
		return;
	}

	// TODO: FADT, DSDT

	// TODO: ACPI enable (section 16.3.1)

	// TODO: syscalls for changing sleep state (shutdown/reboot/reset/sleep/etc.)

	__cio_printf("[acpi] initialize complete\n");
}
