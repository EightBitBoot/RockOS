#ifndef ACPI_H_
#define ACPI_H_

#include "common.h"

bool_t _acpi_checksum_valid(uint8_t *ptr, uint8_t start, uint8_t length);

void _acpi_init(void);

#endif
