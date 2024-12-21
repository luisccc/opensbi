
#include <libfdt.h>
#include <sbi/sbi_error.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/checker/fdt_worldguard.h>
#include <sbi_utils/checker/worldguard.h>
#include <sbi_utils/checker/fdt_wgchecker.h>

static int fdt_parse_wgmemreg_node(const void *fdt, int nodeoffset)
{
    int len, rc;
	u32 val32, i, rcount, wid;
	u64 val64;
	const u32 *val;
    const u32 *wids;
	unsigned long base, order, perm = 0;
    
    /* Read "base" DT property */
	val = fdt_getprop(fdt, nodeoffset, "base", &len);
	if (!val || len != 8)
		return SBI_EINVAL;
	val64 = fdt32_to_cpu(val[0]);
	val64 = (val64 << 32) | fdt32_to_cpu(val[1]);
	base = val64;

	/* Read "order" DT property */
	val = fdt_getprop(fdt, nodeoffset, "order", &len);
	if (!val || len != 4)
		return SBI_EINVAL;
	val32 = fdt32_to_cpu(*val);
	if (val32 < 3 || __riscv_xlen < val32)
		return SBI_EINVAL;
	order = val32;

    /* Read "wids" DT property */
    wids = fdt_getprop(fdt, nodeoffset, "wids", &len);
	if (!wids)
		return 0;

	rcount = (u32)len / (sizeof(u32) * 2);
	for (i = 0; i < rcount; i++) {
        wid = fdt32_to_cpu(wids[2 * i]);

        perm |= ((fdt32_to_cpu(wids[(2 * i) + 1])&0x3) << wid*2);
	}

    rc = worldguard_memregion_init(base, order, perm);

    return rc;
}

static int fdt_wgmem_init(const void *fdt, int nodeoffset)
{
	int noff = nodeoffset, rc;
	
	/* Check all DT nodes */
	while ((noff = fdt_node_offset_by_compatible(fdt, noff,
						"worldguard,memregion")) >= 0) {
		
		rc = fdt_parse_wgmemreg_node(fdt, noff);
		if (rc == SBI_ENODEV)
			continue;
	}

    return 0;
}

int fdt_worldguard_init(const void *fdt)
{
	int rc, poffset;

    worldguard_init();
	
    poffset = fdt_path_offset(fdt, "/worldguard");
	if (poffset < 0)
		return 0;

    rc = fdt_wgmem_init(fdt, poffset);

    if (rc) return rc;
    
	rc = fdt_wgchecker_init(fdt, poffset);

	return rc;
}