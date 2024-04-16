#include <acpi/acpi.h>
#include <acpi/tables/sdt.h>
#include "debug.h"

bool_t _acpi_validate_sdt(struct acpi_sdt_header *header) {
  if (header == NULL) return false;

  // Check sum
  if (!_acpi_checksum_valid((uint8_t *) header, 0, header->length)) {
    WARNING("[acpi] SDT checksum invalid");
    return false;
  }

  return true;
}
