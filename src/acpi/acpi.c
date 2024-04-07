#include <acpi/acpi.h>
#include <acpi/tables/rsdp.h>
#include <acpi/tables/sdt.h>
#include <io/cio.h>
#include <debug.h>

struct acpi_data {
	struct acpi_rsdp* rsdp;

	struct acpi_sdt_root32* rsdt;
	struct acpi_sdt_root64* xsdt;

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
		__cio_printf("[acpi] System is not ACPI-capable\n");
		return;
	}

	// Find and validate RSDT/XSDT
	if (_acpi_data.rsdp->revision >= 2) {
		struct acpi_sdt_root64* xsdt = (struct acpi_sdt_root64 *) _acpi_data.rsdp->xsdt_address;

		if (!_acpi_checksum_valid((uint8_t *) xsdt, 0, xsdt->header.length)) {
			WARNING("[acpi] XSDT checksum invalid, falling through to RSDT");
		} else {
			_acpi_data.xsdt = xsdt;
			__cio_printf("[acpi] XSDT at 0x%x\n", _acpi_data.xsdt);
		}
	}

	if (_acpi_data.xsdt == NULL && _acpi_data.rsdp->revision == 0) {
		struct acpi_sdt_root32* rsdt = (struct acpi_sdt_root32 *) _acpi_data.rsdp->rsdt_address;

		if (!_acpi_checksum_valid((uint8_t *) rsdt, 0, rsdt->header.length)) {
			WARNING("[acpi] RSDT checksum invalid, aborting");
			return;
		} else {
			_acpi_data.rsdt = rsdt;
			__cio_printf("[acpi] RSDT at 0x%x\n", _acpi_data.rsdt);
		}
	}

	// TODO: Search through RSDT/XSDT to find FADT, DSDT

	// TODO: ACPI enable (section 16.3.1)

	// TODO: syscalls for changing sleep state (shutdown/reboot/reset/sleep/etc.)

	__cio_printf("[acpi] initialize complete\n");
}
