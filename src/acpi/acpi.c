#include <acpi/acpi.h>
#include <acpi/tables/rsdp.h>
#include <acpi/tables/sdt.h>
#include <acpi/tables/fadt.h>
#include <acpi/tables/dsdt.h>
#include "support.h"
#include "lib.h"
#include <io/cio.h>
#include <debug.h>

struct acpi_data {
	struct acpi_rsdp *rsdp;

	struct acpi_sdt_root32 *rsdt;
	struct acpi_sdt_root64 *xsdt;

	struct acpi_fadt *fadt;
	// TODO: how to parse AML coded tables dynamically?
	struct acpi_dsdt *dsdt;
};
static struct acpi_data _acpi_data = {0};

static bool_t _acpi_locate_rsdp(void) {
	// Find and ensure RSDP table is valid
	_acpi_data.rsdp = _acpi_get_rsdp_ptr();

	if (_acpi_data.rsdp == NULL) return false;

	_acpi_dbg("RSDP @0x%x", _acpi_data.rsdp);
	return true;
}

// Assumes _acpi_data.rsdp is valid
static bool_t _acpi_locate_root_sdts(void) {
	// Find and validate XSDT (if present)
	if (_acpi_data.rsdp->revision >= 2) {
		struct acpi_sdt_root64* xsdt = (struct acpi_sdt_root64 *) _acpi_data.rsdp->xsdt_address;
		if (_acpi_validate_sdt(&xsdt->header)) {
			// Warn on XSDT on 32-bit systems
			if (sizeof(void *) < 8) {
				_acpi_warn("Using 64-bit tables on 32-bit system. This may cause issues!");
			}

			_acpi_data.xsdt = xsdt;
			_acpi_dbg("XSDT @0x%x", _acpi_data.xsdt);

			return true;
		} else {
			_acpi_warn("XSDT checksum invalid");
		}
	}

	// Find and validate RSDT
	struct acpi_sdt_root32* rsdt = (struct acpi_sdt_root32 *) _acpi_data.rsdp->rsdt_address;
	if (_acpi_validate_sdt(&rsdt->header)) {
		_acpi_data.rsdt = rsdt;
		_acpi_dbg("RSDT @0x%x", _acpi_data.rsdt);

		return true;
	} else {
		_acpi_warn("RSDT checksum invalid");
	}

	// No valid table found
	return false;
}

// Assumes _acpi_data.rsdt or _acpi_data.xsdt is valid
static bool_t _acpi_locate_sdts(void) {
	int num_entries = 0;
	struct acpi_sdt_header **entries;

	// Select XSDT/RSDT to find entries
	if (_acpi_data.xsdt != NULL) {
		_acpi_info("Searching XSDT for SDTs");
		num_entries = (_acpi_data.xsdt->header.length - sizeof(_acpi_data.xsdt->header)) / sizeof(uint64_t);
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		entries = (struct acpi_sdt_header **) _acpi_data.xsdt->entry_addresses;
		#pragma GCC diagnostic pop
	} else if (_acpi_data.rsdt != NULL) {
		_acpi_info("Searching RSDT for SDTs");
		num_entries = (_acpi_data.rsdt->header.length - sizeof(_acpi_data.rsdt->header)) / sizeof(uint32_t);
		entries = (struct acpi_sdt_header **) _acpi_data.rsdt->entry_addresses;
	}

	// Find entries
	for (uint32_t i = 0; i < num_entries; i++) {
		struct acpi_sdt_header *entry = (struct acpi_sdt_header *) entries[i];

		if (_acpi_validate_sdt(entry)) {
			char signature[5] = {0};
			for (uint8_t j = 0; j < 4; j++)
				signature[j] = entry->signature[j];

			_acpi_dbg("SDT (%02d/%02d) @0x%-08x '%s'", (i+1), num_entries, entry, signature);

			if (__memcmp(entry->signature, ACPI_FADT_SIGNATURE, sizeof(ACPI_FADT_SIGNATURE)) == 0) {
				_acpi_data.fadt = (struct acpi_fadt *) entry;
			}
		} else {
			_acpi_dbg("SDT (%02d/%02d) @0x%-08x invalid! ignoring.", (i+1), num_entries, entry);
		}
	}

	// Return error on missing required table
	return (_acpi_data.fadt != NULL);
}

// Assumes FADT is valid.
static bool_t _acpi_parse_dsdt(struct acpi_dsdt *dsdt) {
	if (dsdt == NULL) return false;

	// DSDT checksum
	if (!_acpi_validate_sdt(&dsdt->header)) {
		_acpi_warn("DSDT checksum invalid");
		return false;
	}

	// Signature
	if (__memcmp(dsdt->header.signature, ACPI_DSDT_SIGNATURE, sizeof(ACPI_DSDT_SIGNATURE)) != 0) {
		_acpi_warn("DSDT invalid signature");
		return false;
	}

	_acpi_data.dsdt = dsdt;
	_acpi_dbg("DSDT @0x%x", _acpi_data.dsdt);

	char *dsdt_ptr = _acpi_data.dsdt->definition_block;
	while (true) {
		__cio_putchar(*(dsdt_ptr++));
		__delay(5);
	}

	return true;
}

static bool_t _acpi_enable(void) {
	// See: ACPI Specification (v6.5) Section 16.3.1
	if ((__inw(_acpi_data.fadt->pm1a_cnt_blk) & 1) == 0) {
		_acpi_dbg("Issuing ACPI enable command. Waiting for transition to ACPI mode...");
		__outb(_acpi_data.fadt->smi_cmd, _acpi_data.fadt->acpi_enable);
	}

	// TODO: Give up after a while?
	while ((__inw(_acpi_data.fadt->pm1a_cnt_blk) & 1) == 0);

	return true;
}

void _acpi_init(void) {
	_acpi_info("Initializing");

	// Find and ensure RSDP table is valid
	if (!_acpi_locate_rsdp()) {
		_acpi_info("System is not ACPI-capable. Failed to find valid RSDP. Initialization failed.");
		return;
	}

	if (!_acpi_locate_root_sdts()) {
		_acpi_info("Found RSDP, but failed to find valid RSDT/XSDT! Initialization failed.");
		return;
	}

	// Search through RSDT/XSDT to find FADT, DSDT
	if (!_acpi_locate_sdts()) {
		_acpi_info("Failed to locate essential System Descriptor Tables. Initialization failed.");
		return;
	}

	// Parse DSDT
	_acpi_dbg("Trying extended DSDT address");
	if (!_acpi_parse_dsdt((struct acpi_dsdt *) _acpi_data.fadt->x_dsdt)) {
		// TODO: This fails. Is the DSDT definition wrong?
		_acpi_info("Failed to parse extended DSDT.");
	}

	if (_acpi_data.dsdt == NULL) {
		_acpi_dbg("Trying regular DSDT address");
		if (!_acpi_parse_dsdt((struct acpi_dsdt *) _acpi_data.fadt->dsdt)) {
			_acpi_info("Failed to parse regular DSDT. Initialization failed.");
			return;
		}
	}

	// Transition from legacy to APCI mode
	if (!_acpi_enable()) {
		_acpi_info("Failed to transition from legacy mode to ACPI mode. Initialization failed.");
		return;
	}
	_acpi_info("ACPI mode enabled");

	// TODO: ACPI interrupts + enable them?

	// TODO: syscalls for changing sleep state (shutdown/reboot/reset/sleep/etc.)

	_acpi_info("Initialization complete");
}
