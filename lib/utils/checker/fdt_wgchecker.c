
#include <libfdt.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_console.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/checker/fdt_wgchecker.h>
#include <sbi_utils/checker/wgchecker.h>

int fdt_wgchecker_init(const void *fdt, int nodeoffset)
{
	int noff = -1, rc;
	struct platform_wgchecker_data checker = { 0 };

	/* Check all DT nodes */
	while ((noff = fdt_node_offset_by_compatible(fdt, noff,
						"worldguard,checker")) >= 0) {
		
		rc = fdt_parse_wgchecker_node(fdt, noff, &checker);
		if (rc == SBI_ENODEV)
			continue;

		rc = wgchecker_init(&checker);
	}

	return rc;
}

int fdt_parse_wgchecker_node(const void *fdt, int nodeoffset,
				      struct platform_wgchecker_data *wgchecker)
{
	int rc;
	uint64_t reg_addr, reg_size;

	if (nodeoffset < 0 || !wgchecker || !fdt)
		return SBI_ENODEV;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_addr || !reg_size)
		return SBI_ENODEV;
	wgchecker->addr = reg_addr;

	return 0;
}