#include <acpi/acpi.h>
#include <acpi/tables/rsdp.h>
#include <acpi/tables/sdt.h>
#include <acpi/tables/fadt.h>
#include "support.h"
#include "lib.h"
#include <io/cio.h>
#include <debug.h>

struct acpi_data {
	struct acpi_rsdp *rsdp;

	struct acpi_sdt_root32 *rsdt;
	struct acpi_sdt_root64 *xsdt;

	struct acpi_fadt *fadt;
	// TODO: how to load and store AML coded tables dynamically?
};
static struct acpi_data _acpi_data = {0};

void _acpi_init(void) {
	__cio_printf("[acpi] initialize\n");

	// Find and ensure RSDP table is valid
	_acpi_data.rsdp = _acpi_get_rsdp_ptr();
	if (_acpi_data.rsdp == NULL) {
		__cio_printf("[acpi] System is not ACPI-capable\n");
		return;
	}

	__cio_printf("[acpi] RSDP @0x%x\n", _acpi_data.rsdp);

	// Find and validate XSDT (if present)
	if (_acpi_data.rsdp->revision >= 2) {
		struct acpi_sdt_root64* xsdt = (struct acpi_sdt_root64 *) _acpi_data.rsdp->xsdt_address;
		if (!_acpi_validate_sdt(&xsdt->header)) {
			WARNING("[acpi] XSDT checksum invalid");
		} else {
			// Warn on XSDT on 32-bit systems
			if (sizeof(void *) < 8) {
				WARNING("[acpi] Using 64-bit tables on 32-bit system. This may cause issues!");
			}

			_acpi_data.xsdt = xsdt;
			__cio_printf("[acpi] XSDT @0x%x\n", _acpi_data.xsdt);
		}
	}

	// Find and validate RSDT
	struct acpi_sdt_root32* rsdt = (struct acpi_sdt_root32 *) _acpi_data.rsdp->rsdt_address;
	if (!_acpi_validate_sdt(&rsdt->header)) {
		WARNING("[acpi] RSDT checksum invalid");
	} else {
		_acpi_data.rsdt = rsdt;
		__cio_printf("[acpi] RSDT @0x%x\n", _acpi_data.rsdt);
	}

	// Search through RSDT/XSDT to find FADT, DSDT
	int num_entries = 0;
	struct acpi_sdt_header **entries;

	if (_acpi_data.xsdt != NULL) {
		__cio_printf("[acpi] Searching XSDT for SDTs\n");
		num_entries = (_acpi_data.xsdt->header.length - sizeof(_acpi_data.xsdt->header)) / sizeof(uint64_t);
		entries = (struct acpi_sdt_header **) _acpi_data.xsdt->entry_addresses;
	} else if (_acpi_data.rsdt != NULL) {
		__cio_printf("[acpi] Searching RSDT for SDTs\n");
		num_entries = (_acpi_data.rsdt->header.length - sizeof(_acpi_data.rsdt->header)) / sizeof(uint32_t);
		entries = (struct acpi_sdt_header **) _acpi_data.rsdt->entry_addresses;
	} else {
		WARNING("[acpi] Unable to find good RSDT or XSDT, exiting");
		return;
	}

	for (uint32_t i = 0; i < num_entries; i++) {
		struct acpi_sdt_header *entry = (struct acpi_sdt_header *) entries[i];
		__cio_printf("[acpi] SDT (%02d/%02d) @0x%-08x ", (i + 1), num_entries, entry);

		if (_acpi_validate_sdt(entry)) {
			__cio_printf("\"");

			for (uint8_t j = 0; j < 4; j++)
				__cio_printf("%c", entry->signature[j]);

			__cio_printf("\"\n");

			if (__memcmp(entry->signature, ACPI_FADT_SIGNATURE, sizeof(ACPI_FADT_SIGNATURE)) == 0) {
				_acpi_data.fadt = (struct acpi_fadt *) entry;
			}
		} else {
			__cio_printf("invalid! ignoring.\n");
		}

		__delay(25);
	}

	// TODO: ACPI enable (section 16.3.1)
	if (_acpi_data.fadt == NULL) {
		WARNING("[acpi] Unable to locate FADT, aborting");
	}

	// TODO: syscalls for changing sleep state (shutdown/reboot/reset/sleep/etc.)

	__cio_printf("[acpi] initialize complete\n");
	while(true) {}
}
