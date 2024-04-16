#include <acpi/acpi.h>
#include <acpi/tables/sdt.h>

/**
 * @struct acpi_fadt_iapc
 * @brief Fixed ACPI Description Table IA-PC Architecture Boot Flags
 * @sa ACPI Specification (v6.5) Section 5.2.9.3
 */
struct acpi_fadt_iapc {
	uint8_t legacy_devices;
	uint8_t _8042;
	uint8_t vga_not_present;
	uint8_t msi_not_supported;
	uint8_t pcie_aspm_controls;
	uint8_t cmos_rtc_not_present;
	uint8_t reserved[10];
} __attribute__((packed));

/**
 * @struct acpi_fadt_arm
 * @brief Fixed ACPI Description Table ARM Architecture Boot Flags
 * @sa ACPI Specification (v6.5) Section 5.2.9.4
 */
struct acpi_fadt_arm {
	uint8_t psci_compliant;
	uint8_t psci_use_hvc;
	uint8_t reserved[14];
} __attribute__((packed));

#define ACPI_FADT_SIGNATURE "FACP"
/**
 * @struct acpi_fadt
 * @brief Fixed ACPI Description Table
 * @sa ACPI Specification (v6.5) Section 5.2.9
 */
struct acpi_fadt {
	struct acpi_sdt_header header;

	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t reserved1; // Must be 0 or 1
	uint8_t preferred_pm_profile;
	uint16_t sci_int;
	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_cnt;
	uint32_t pm1a_evt_blk;
	uint32_t pm1b_evt_blk;
	uint32_t pm1a_cnt_blk;
	uint32_t pm1b_cnt_blk;
	uint32_t pm2_cnt_blk;
	uint32_t pm_tmr_blk;
	uint32_t gpe0_blk;
	uint32_t gpe1_blk;
	uint8_t pm1_evt_len;
	uint8_t pm1_cnt_len;
	uint8_t pm2_cnt_len;
	uint8_t pm_tmr_len;
	uint8_t gpe0_blk_len;
	uint8_t gpe1_blk_len;
	uint8_t gpe1_base;
	uint8_t cst_cnt;
	uint16_t p_lvl2_lat;
	uint16_t p_lvl3_lat;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alrm;
	uint8_t mon_alrm;
	uint8_t century;
	struct acpi_fadt_iapc iapc_boot_arch;
	uint8_t reserved2; // Must be 0
	uint32_t flags;
	struct acpi_generic_address reset_reg;
	uint8_t reset_value;
	struct acpi_fadt_arm arm_boot_arch;
	uint8_t revision_minor;
	uint64_t x_firmware_ctrl;
	uint64_t x_dsdt;
	struct acpi_generic_address x_pm1a_evt_blk;
	struct acpi_generic_address x_pm1b_evt_blk;
	struct acpi_generic_address x_pm1a_cnt_blk;
	struct acpi_generic_address x_pm1b_cnt_blk;
	struct acpi_generic_address x_pm2_cnt_blk;
	struct acpi_generic_address x_pm_tmr_blk;
	struct acpi_generic_address x_gpe0_blk;
	struct acpi_generic_address x_gpe1_blk;
	struct acpi_generic_address sleep_control_reg;
	struct acpi_generic_address sleep_status_reg;
	uint64_t hypervisor_vendor_id;
	uint8_t wbinvd;
	uint8_t wbinvd_flash;
	uint8_t proc_c1;
	uint8_t p_lvl2_up;
	uint8_t pwr_button;
	uint8_t slp_button;
	uint8_t fix_rtc;
	uint8_t rtc_s4;
	uint8_t tmr_val_ext;
	uint8_t dck_cap;
	uint8_t reset_reg_sup;
	uint8_t sealed_case;
	uint8_t headless;
	uint8_t cpu_sw_slp;
	uint8_t pci_exp_wak;
	uint8_t use_platform_clock;
	uint8_t s4_rtc_sts_valid;
	uint8_t remote_power_on_capable;
	uint8_t force_apic_cluster_model;
	uint8_t force_apic_physical_destination_mode;
	uint8_t hw_reduced_acpi;
	uint8_t low_power_s0_idle_capable;
	uint16_t persistent_cpu_caches;
	uint64_t reserved3;
} __attribute__((packed));
