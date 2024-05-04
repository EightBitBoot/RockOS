#include <acpi/aml.h>
#include <kern/support.h>
#include <libc/lib.h>
#include <io/cio.h>

/**
 * This method parses the DefName AML rule. The top-level rule definition follows:
 * 	DefName := NameOp NameString DataRefObject
 *
 * @sa ACPI Specification (v6.5) Section 20.2.2 "Name Objects Encoding"
 */
bool_t _acpi_aml_parse_defname(uint8_t *aml, char *expected) {
	if (aml == NULL) {
		__cio_printf("%s warn: passed NULL name to name op parser\n", __func__);
		return false;
	}

	// TODO: It would be a good idea to check bounds here too, but not that important assuming AML is well-formed

	/*** Parse NameOp ***/
	// NameOp := 0x08
	if (*(aml++) != ACPI_AML_OP_NAME) {
		__cio_printf("%s warn: passed non-name op to name op parser\n", __func__);
		return false;
	}

	/*** Parse NameString ***/
	// NameString := <rootchar namepath> | <prefixpath namepath>

	// rootchar vs prefix path
	switch (*aml) {
		// RootChar := ‘' and ‘' := 0x5C
		// TODO: the ‘' should be ‘\', the spec is wrong. I emailed the UEFI forum to tell them about this.
		case '\\':
			// TODO: wtf is root path vs prefix path?
			aml++;
			//__cio_printf("root path\n");
			break;
		// PrefixPath := Nothing | <’^’ prefixpath>
		case '^':
			// TODO: wtf is root path vs prefix path?
			aml++;
			// Fall through to common prefix path handling logic
		default:
			//__cio_printf("default ");
			// This is a prefix path, but we shouldn't have consumed a character
			// Fall through to common prefix path handling logic
			aml = aml; // ignore compiler error
	}

	// NamePath := NameSeg | DualNamePath | MultiNamePath | NullName
	uint8_t n_namesegs = 1;
	if (*aml == 0x2E) { // DualNamePrefix := 0x2E
		// DualNamePath := DualNamePrefix NameSeg NameSeg
		//__cio_printf("dual name path!\n");
		aml++;
		n_namesegs = 2;
	} else if (*aml == 0x2F) { // MultiNamePrefix := 0x2F
		// MultiNamePath := MultiNamePrefix SegCount NameSeg(SegCount)
		//__cio_printf("multinamepath!\n");
		aml++; // account for prefix
		n_namesegs = *(aml++); // SegCount := ByteData
	} else if (*aml == 0x00) { // NullName := 0x00
		//__cio_printf("null name!\n");
		// Check if the expected name is a null character, return result
		return (*expected == '\0');
	}

	__cio_printf("n(%u): ", n_namesegs);

	// NameSeg := <leadnamechar namechar namechar namechar>
	// Notice that NameSegs shorter than 4 characters are filled with trailing underscores (‘_’s).
	char nameseg[5] = {0};
	for (uint8_t _ = 0; _ < n_namesegs; _++) {
		for (uint8_t j = 0; j < 4; j++) {
			char c = *(aml++);

			// Sanity check name
			// ‘A’-‘Z’ := 0x41 - 0x5A, ‘_’ := 0x5F, ‘0’-‘9’ := 0x30 - 0x39
			bool_t leadnamechar = (c == '_' || (c >= 'A' && c <= 'Z')); // LeadNameChar := ‘A’-‘Z’ | ‘_’
			bool_t digitchar = (c >= '0' && c <= '9'); // DigitChar := ‘0’ - ‘9’
			bool_t namechar = digitchar || leadnamechar; // NameChar := DigitChar | LeadNameChar

			if (j == 0 && !leadnamechar) {
				//__cio_printf("warn %s: leadchar cannot be %c (0x%x)\n", __func__, c, c);
				// TODO: how to handle?
			} else if (!namechar) {
				//__cio_printf("warn %s: namechar cannot be %c (0x%x)\n", __func__, c, c);
				// TODO: how to handle?
			}

			nameseg[j] = c;
		}

		__cio_printf("%s, ", nameseg);

		if (__strcmp(nameseg, expected) == 0) {
			return true;
		}
	}

	__cio_printf(".\n");

	// TODO: either parse or how do we tell parser which character we consumed last?
	/*** TODO: Parse DataRefObject ***/

	return false;
}

bool_t _acpi_aml_find_value(uint8_t *start, uint32_t length, char *key) {
	for (uint32_t i = 0; i < length; i++) {
		uint8_t c = start[i];

		switch (c) {
			// DefName := NameOp NameString DataRefObject
			case ACPI_AML_OP_NAME:
				// TODO: print detected NameOp
				if (_acpi_aml_parse_defname((start + i), key)) {
					__cio_printf("\n\n\nFound key %s!\n\n\n", key);
					//while(true) {}
					return true;
				}
				break;
			/*case ACPI_AML_OP_EXT:
				i++;
				 TODO: Extended operations
				break;*/
			// TODO: unimplemented/unknown operator default case
		}
	}

	return false;
}
