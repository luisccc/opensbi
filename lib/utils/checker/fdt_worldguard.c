
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
	unsigned long base, order, perm = 0, permh = 0, permh2 = 0, permh3 = 0;
    
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
		
		if(wid > 95) {
			wid -= 96;
			permh3 |= ((fdt32_to_cpu(wids[(2 * i) + 1])&0x3) << wid*2);
		} else if(wid > 63) {
			wid -= 64;
			permh2 |= ((fdt32_to_cpu(wids[(2 * i) + 1])&0x3) << wid*2);
		} else if(wid > 31) {
			wid -= 32;
			permh |= ((fdt32_to_cpu(wids[(2 * i) + 1])&0x3) << wid*2);
		} else
			perm |= ((fdt32_to_cpu(wids[(2 * i) + 1])&0x3) << wid*2);
	}

    rc = worldguard_memregion_init(base, order, perm, permh, permh2, permh3);

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

int fdt_wghart_init(const void *fdt)
{
	const u32 *val;
	u32 hartid, wid = 0;
	u64 widdeleg = 0, widdelegh = 0;
	int err, len, cpus_offset, cpu_offset;

	/* Sanity checks */
	if (!fdt)
		return SBI_EINVAL;

	/* Find /cpus DT node */
	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		return cpus_offset;

	fdt_for_each_subnode(cpu_offset, fdt, cpus_offset) {
		err = fdt_parse_hart_id(fdt, cpu_offset, &hartid);
		if (err)
			continue;

		if (!fdt_node_is_enabled(fdt, cpu_offset))
			continue;

		val = fdt_getprop(fdt, cpu_offset, "lp-wid", &len);
		if (!(!val || len != 4))
			wid = fdt32_to_cpu(*val);

		val = fdt_getprop(fdt, cpu_offset, "widdeleg", &len);
		if(val){
			widdeleg = fdt32_to_cpu(val[0]);
			if(len == 8)
				widdeleg = (widdeleg << 32) | fdt32_to_cpu(val[1]);
		}
		
		val = fdt_getprop(fdt, cpu_offset, "widdelegh", &len);
		if(val){
			widdelegh = fdt32_to_cpu(val[0]);
			if(len == 8)
				widdelegh = (widdelegh << 32) | fdt32_to_cpu(val[1]);
		}

		break;
	}

	worldguard_hart_init(wid, widdeleg, widdelegh);
	
	return 0;
}

int fdt_worldguard_init(const void *fdt, bool cold_boot)
{
	int rc, poffset;

	if(cold_boot)
    	worldguard_init();
	
    poffset = fdt_path_offset(fdt, "/worldguard");
	if (poffset < 0)
		return 0;

	rc = fdt_wghart_init(fdt);
	if (rc) return rc;

	if(cold_boot){
		rc = fdt_wgmem_init(fdt, poffset);
		if (rc) return rc;
		
		rc = fdt_wgchecker_init(fdt, poffset);
	}

	return rc;
}