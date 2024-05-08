#include <acpi/acpi.h>
#include <acpi/aml.h>
#include <kern/support.h>
#include <debug.h>
#include <libc/lib.h>
#include <io/cio.h>

/**
 * This method parses the DataRefObject AML rule. The top-level rule definition follows:
 *
 * 	DataRefObject := DataObject | ObjectReference
 *
 * @sa ACPI Specification (v6.5) Section 20.2.3 "Data Objects Encoding"
 */
static bool_t _acpi_aml_parse_datarefobject(uint8_t *aml, uint8_t *output) {
	if (aml == NULL) {
		_acpi_warn("passed NULL aml to DataRefObject parser");
		return false;
	}

	/*** Parse DataObject ***/
	// DataObject := ComputationalData | DefPackage | DefVarPackage

	// TODO ComputationalData

	// DefPackage
	if (*aml == ACPI_AML_OP_PACKAGE) { // PackageOp := 0x12
		// DefPackage := PackageOp PkgLength NumElements PackageElementList

		// PackageOp
		aml++;

		// PkgLength (this is cursed)
		uint32_t pkglength;
		uint8_t n_bytes = (*aml) >> 6; // bits 6-7 are n bytes

		if (n_bytes == 0) {
			pkglength = (*(aml++)) & 63; // bits 0-5
		} else {
			pkglength = (*(aml++)) & 15; // bits 0-3
			for (uint8_t i = 0; i < n_bytes; i++) {
				pkglength = pkglength << 8;
				pkglength += *(aml++);
			}
		}

		// NumElements
		uint8_t n_elements = *(aml++);

		// PackageElementList
		// TODO: Packages can store more than bytes, but we only need to parse bytes
		if (*(aml++) != 0x0A) _acpi_warn("Expected ByteData but got unknown package data type");
		for (int i = 0; i < n_elements; i++) {
			output[i] = *(aml++);
		}
	}

	// TODO: DefVarPackage

	/*** TODO: Parse ObjectReference ***/

	return false;
}

/**
 * This method parses the DefName AML rule. The top-level rule definition follows:
 *
 * 	DefName := NameOp NameString DataRefObject
 *
 * @sa ACPI Specification (v6.5) Section 20.2.2 "Name Objects Encoding"
 */
static bool_t _acpi_aml_parse_defname(uint8_t *aml, char *expected, uint8_t *output) {
	if (aml == NULL) {
		_acpi_warn("passed NULL aml to NameOp parser");
		return false;
	}

	// TODO: It would be a good idea to check bounds here too, but not that important assuming AML is well-formed

	/*** Parse NameOp ***/
	// NameOp := 0x08
	if (*(aml++) != ACPI_AML_OP_NAME) {
		_acpi_warn("passed non-NameOp construction to NameOp parser");
		return false;
	}

	/*** Parse NameString ***/
	// NameString := <rootchar namepath> | <prefixpath namepath>

	// rootchar vs prefix path
	switch (*aml) {
		// RootChar := ‘' and ‘' := 0x5C
		// TODO: the ‘' should be ‘\', the spec is wrong. I emailed the UEFI forum to tell them about this.
		case '\\':
			// TODO: use this for something, we don't need it for shutdown
			_acpi_dbg("rootchar");
			aml++;
			break;
		// PrefixPath := Nothing | <’^’ prefixpath>
		case '^':
			aml++;
			// Fall through to common prefix path handling logic
		default:
			// TODO: use this for something, we don't need it for shutdown
			_acpi_dbg("prefixpath");

	}

	// NamePath := NameSeg | DualNamePath | MultiNamePath | NullName
	uint8_t n_namesegs = 1;
	if (*aml == 0x2E) { // DualNamePrefix := 0x2E
		// DualNamePath := DualNamePrefix NameSeg NameSeg
		_acpi_dbg("Parsing DualNamePath");
		aml++;
		n_namesegs = 2;
	} else if (*aml == 0x2F) { // MultiNamePrefix := 0x2F
		// MultiNamePath := MultiNamePrefix SegCount NameSeg(SegCount)
		_acpi_dbg("Parsing MultiNamePath");
		aml++; // account for prefix
		n_namesegs = *(aml++); // SegCount := ByteData
	} else if (*aml == 0x00) { // NullName := 0x00
		_acpi_dbg("Parsing NullName");
		// Check if the expected name is a null character, return result
		return (*expected == '\0');
	}

	// NameSeg := <leadnamechar namechar namechar namechar>
	// Notice that NameSegs shorter than 4 characters are filled with trailing underscores (‘_’s).
	char nameseg[5] = {0};
	bool_t namefound = false;
	for (uint8_t _ = 0; _ < n_namesegs; _++) {
		for (uint8_t j = 0; j < 4; j++) {
			char c = *(aml++);

			// Sanity check name
			// ‘A’-‘Z’ := 0x41 - 0x5A, ‘_’ := 0x5F, ‘0’-‘9’ := 0x30 - 0x39
			bool_t leadnamechar = (c == '_' || (c >= 'A' && c <= 'Z')); // LeadNameChar := ‘A’-‘Z’ | ‘_’
			bool_t digitchar = (c >= '0' && c <= '9'); // DigitChar := ‘0’ - ‘9’
			bool_t namechar = digitchar || leadnamechar; // NameChar := DigitChar | LeadNameChar

			if (j == 0 && !leadnamechar) {
				_acpi_warn("invalid leadchar detected");
				// TODO: how to handle?
			} else if (!namechar) {
				_acpi_warn("invalid namechar detected");
				// TODO: how to handle?
			}

			nameseg[j] = c;
		}

		_acpi_dbg("Found name '%s'", nameseg);
		if (__strcmp(nameseg, expected) == 0) {
			_acpi_dbg("Found match for %s", nameseg);
			namefound = true;
		}
	}

	if (!namefound) return false;

	/*** Parse DataRefObject ***/
	_acpi_aml_parse_datarefobject(aml++, output);

	return true;
}

bool_t _acpi_aml_find_value(uint8_t *start, uint32_t length, char *key, uint8_t *output) {
	for (uint32_t i = 0; i < length; i++) {
		uint8_t c = start[i];

		switch (c) {
			// DefName := NameOp NameString DataRefObject
			case ACPI_AML_OP_NAME:
				if (_acpi_aml_parse_defname((start + i), key, output)) {
					_acpi_dbg("Successfully found value of '%s'!", key);
					return true;
				}
				break;
		}
	}

	return false;
}
