#include <acpi/acpi.h>

bool_t _acpi_checksum_valid(uint8_t *ptr, uint8_t start, uint8_t length) {
	uint8_t checksum = 0;

	for (uint8_t i = start; i < length; i++) {
		checksum += *(ptr + i);
	}

	return checksum == 0;
}
