#ifndef ACPI_AML_H_
#define ACPI_AML_H_

#include "common.h"

enum acpi_aml_opcodes {
	ACPI_AML_OP_NAME = 0x08,
	ACPI_AML_OP_PACKAGE = 0x12,
	ACPI_AML_OP_EXT = 0x5B,
};
typedef uint8_t acpi_aml_opcode;

bool_t _acpi_aml_find_value(uint8_t *start, uint32_t length, char *key, uint8_t *output);

#endif
