#include <acpi/acpi.h>

bool_t _acpi_checksum_valid(uint8_t *ptr, uint32_t start, uint32_t length) {
	uint8_t checksum = 0;

	if (ptr == NULL)
		return false;

	for (uint32_t i = start; i < length; i++) {
		checksum += *(ptr + i);
	}

	return checksum == 0;
}
