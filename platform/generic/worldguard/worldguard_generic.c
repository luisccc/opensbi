
#include <libfdt.h>
#include <platform_override.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_heap.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_string.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_tlb.h>
#include <sbi/sbi_console.h>
#include <sbi_utils/fdt/fdt_domain.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_pmu.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>
#include <sbi_utils/irqchip/imsic.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/timer/fdt_timer.h>
#include <sbi_utils/ipi/fdt_ipi.h>
#include <sbi_utils/reset/fdt_reset.h>
#include <sbi_utils/checker/fdt_worldguard.h>
#include <sbi_utils/serial/semihosting.h>
#include <sbi/sbi_domain.h>

static int worldguard_generic_final_init(bool cold_boot, void *fdt,
				     const struct fdt_match *match)
{
	if (!cold_boot)
		return 0;

	fdt_worldguard_init(fdt);
	sbi_printf("WorldGuard: Final Init\n");

	return 0;
}

static const struct fdt_match worldguard_generic_match[] = {
	{ .compatible = "worldguard,generic" },
	{ },
};

const struct platform_override worldguard_generic = {
	.match_table = worldguard_generic_match,
	.final_init = worldguard_generic_final_init,
};
