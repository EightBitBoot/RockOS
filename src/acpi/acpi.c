#include <acpi/acpi.h>
#include <acpi/aml.h>
#include <acpi/tables/rsdp.h>
#include <acpi/tables/sdt.h>
#include <acpi/tables/fadt.h>
#include <acpi/tables/dsdt.h>
#include <kern/support.h>
#include <libc/lib.h>
#include <io/cio.h>
#include <debug.h>

struct acpi_data {
	struct acpi_rsdp *rsdp;

	struct acpi_sdt_root32 *rsdt;
	struct acpi_sdt_root64 *xsdt;

	struct acpi_fadt *fadt;
	struct acpi_dsdt *dsdt;

	// AML-sourced data
	uint8_t _S5;
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

	// Signature
        if (__memcmp(dsdt->header.signature, ACPI_DSDT_SIGNATURE, sizeof(ACPI_DSDT_SIGNATURE)) != 0) {
                _acpi_warn("DSDT invalid signature");
                return false;
        }

	// DSDT checksum
	if (!_acpi_validate_sdt(&dsdt->header)) {
		_acpi_warn("DSDT checksum invalid");
		return false;
	}

	_acpi_data.dsdt = dsdt;
	_acpi_dbg("DSDT @0x%x", _acpi_data.dsdt);

	uint8_t *aml = _acpi_data.dsdt->definition_block;
	uint32_t aml_len = _acpi_data.dsdt->header.length - sizeof(_acpi_data.dsdt->header);

	uint8_t buffer[32]; // not expecting more than 4 bytes
	if (!_acpi_aml_find_value(aml, aml_len, "_S5_", buffer)) {
		_acpi_warn("Unable to find or parse _S5_ from DSDT");
		return false;
	}
	_acpi_data._S5 = buffer[0] & 7; // Only first byte matters and only first 3 bits
	_acpi_info("Parsed _S5 value %u from DSDT", _acpi_data._S5);

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

void _acpi_command(enum _acpi_commands cmd) {
	// TODO: Validate assumptions (_acpi_data setup, etc)

	switch (cmd) {
		case ACPI_COMMAND_SHUTDOWN:
			// See: ACPI Specification (v6.5) Section 16.1.6
			_acpi_info("shutting down");

			// TODO: _TTS(S5).
			// Note: The motherboard in the lab does not provide _TTS, likely added in a more recent standard.

			// TODO: _PTS
			// Note: This requires advances AML parsing and interpreting and is not required by the lab's motherboard.

			// TODO: Writes the waking vector into the FACS table in memory.
			// Note: This operating system is not configured to wake, so we don't need to provide a vector.

			// If not a HW-reduced ACPI platform, OSPM clears the WAK_STS in the PM1a_STS and PM1b_STS registers. On HW-reduced ACPI platforms, OSPM clears the WAK_STS bit in the Sleep Status Register.
			_acpi_dbg("Clearing WAK_STS from PM1a and PM1b status registers");
			int pm1a_evt_blk = __inw(_acpi_data.fadt->pm1a_evt_blk);
			pm1a_evt_blk = pm1a_evt_blk | (1 << 15); // WAK_STS
			__outw(_acpi_data.fadt->pm1a_evt_blk, pm1a_evt_blk);

			int pm1b_evt_blk = __inw(_acpi_data.fadt->pm1b_evt_blk);
			pm1b_evt_blk = pm1b_evt_blk | (1 << 15); // WAK_STS;
			__outw(_acpi_data.fadt->pm1b_evt_blk, pm1b_evt_blk);

			// If not entering an S4BIOS state, and not a HW-reduced ACPI platform, then OSPM writes SLP_TYPa (from the associated sleeping object) with the SLP_ENa bit set to the PM1a_CNT register.
			_acpi_dbg("Writing SLP_TYPx and SLP_ENx to PM1a control register");
			int pm1a_cnt_blk = __inw(_acpi_data.fadt->pm1a_cnt_blk);
			pm1a_cnt_blk = pm1a_cnt_blk | (1 << 13) | (_acpi_data._S5 << 10);
			__outw(_acpi_data.fadt->pm1a_cnt_blk, pm1a_cnt_blk);

			// OSPM writes SLP_TYPb with the SLP_EN bit set to the PM1b_CNT register, or writes the HW-reduced ACPI Sleep Type value and the SLP_EN bit to the Sleep Control Register.
			_acpi_dbg("Writing SLP_TYPx and SLP_ENx to PM1b control register");
			int pm1b_cnt_blk = __inw(_acpi_data.fadt->pm1b_cnt_blk);
			pm1b_cnt_blk = pm1b_cnt_blk | (1 << 13) | (_acpi_data._S5 << 10);
			__outw(_acpi_data.fadt->pm1b_cnt_blk, pm1b_cnt_blk);

			// Wait until shutdown happens
			_acpi_dbg("Busy looping CPU until shutdown");
			while (true);
			break;
		case ACPI_COMMAND_REBOOT:
			_acpi_info("rebooting");
			if (_acpi_data.fadt->header.revision < 2) {
				_acpi_dbg("FADT version %u does not support reset register!", _acpi_data.fadt->header.revision);
				// TODO: crash the system in another way
			}

			_acpi_dbg("Reset! ID 0x%x ADDR 0x%x VAL 0x%x", _acpi_data.fadt->reset_reg.addr_space_id, _acpi_data.fadt->reset_reg.addr, _acpi_data.fadt->reset_value);
			switch (_acpi_data.fadt->reset_reg.addr_space_id) {
				case ACPI_ADDR_SPACE_SYSTEM_MEM:
					_acpi_dbg("Issuing reset via system memory");
					*((uint8_t *) _acpi_data.fadt->reset_reg.addr) = _acpi_data.fadt->reset_value;
					break;
				case ACPI_ADDR_SPACE_SYSTEM_IO:
					_acpi_dbg("Issuing reset via system IO");
					// TODO: Test. The lab computers use SYSTEM_MEM
					__outb(_acpi_data.fadt->reset_reg.addr, _acpi_data.fadt->reset_value);
					break;
				case ACPI_ADDR_SPACE_PCI_CONFIG:
					_acpi_dbg("Issuing reset via PCI configuration");
					// TODO: Implement and test. The lab computers use SYSTEM_MEM
					break;
				default:
					_acpi_info("Invalid address space ID %u for FADT reset", _acpi_data.fadt->reset_reg.addr_space_id);
			}

			// OSPM should execute spin loops on all CPUs once writing reset register.
			_acpi_dbg("Busy looping CPU until reboot");
			while(true);

			break;
		default:
			_acpi_info("Invalid command sent to syscall: %u", cmd);
	}
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
