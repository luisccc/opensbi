#include <sbi_utils/iopmp/iopmp.h>

#include <sbi/sbi_domain.h>
#include <sbi/sbi_console.h>
#include <sbi/riscv_io.h>

static struct iopmp_data *iopmp;

void iopmp_init_data(struct iopmp_data *iopmp_data)
{
    iopmp = iopmp_data;
}

void iopmp_configure(void)
{
    u32 i;
    u32* rrid;
    u64  bmap = 0;
    struct sbi_domain_memregion *reg;
	struct sbi_iodomain *dom;
    unsigned int entry_flags = 0, entry_count = 0, rrid_count = 0;

	sbi_iodomain_for_each(i, dom) {
        sbi_printf("IO Domain %d \n", i);
        entry_count = 0;
        rrid_count  = 0;

        // Setup regions
		sbi_domain_for_each_memregion(dom, reg) {
            if (reg->flags & SBI_DOMAIN_MEMREGION_READABLE)
			    entry_flags |= IOPMP_ACCESS_READ;
            if (reg->flags & SBI_DOMAIN_MEMREGION_WRITEABLE)
			    entry_flags |= IOPMP_ACCESS_WRITE;
            if (reg->flags & SBI_DOMAIN_MEMREGION_EXECUTABLE)
			    entry_flags |= IOPMP_ACCESS_EXEC;
            if (reg->flags & SBI_DOMAIN_MEMREGION_AMODE)
			    entry_flags |= SBI_DOMAIN_MEMREGION_EXTRACT_AMODE(reg->flags);

            sbi_printf("Entry %d -> Flags: %x, Addr: %lx, Order: %lx \n", entry_count, entry_flags, reg->base, reg->order);
            iopmp_entry_set(entry_count++, entry_flags, reg->base, reg->order);
        }

        // Setup domains MDCFG
        iopmp_mdcfg_config(i, entry_count);
        sbi_printf("MD %d -> T: %d\n", i, entry_count);

        // Setup RRID MDCFG
        sbi_iodomain_for_each_rrid(dom, rrid, rrid_count){
            bmap = (1 << i);    // Create a bitmap value with the current value of md
            sbi_printf("Pushing bmap: %lx to rrid %d \n", bmap, *rrid);
            iopmp_srcmd_config(*rrid, bmap, 0);
        }

	}
}

void iopmp_enable(void)
{
    u32 config = 0x80000000; // Current spec the enable bit is the last bit of the HWCFG
    u32 *addr;

    addr = (void *)iopmp_inst_hwcfg0(iopmp);
    writel(config, addr);
}

int iopmp_entry_set(unsigned int n, unsigned long prot, unsigned long addr,
	    unsigned long log2len)
{
    u64 *entry_addr;
    u32 *entry_config;
    entry_addr = (void *)iopmp_inst_to_entry(iopmp, n);
    entry_config = (void *)iopmp_inst_to_entry_cfg(iopmp, n);

	// int pmpcfg_csr, pmpcfg_shift, pmpaddr_csr;
	// unsigned long cfgmask, pmpcfg;
	unsigned long addrmask, pmpaddr;

	/* encode PMP config */
    if (prot & IOPMP_MODE_NA){   // Is it naturally aligned? Verify which
        prot |= (log2len == 2) ? IOPMP_MODE_NA4 : IOPMP_MODE_NAPOT;
    }

	/* encode PMP address */
	if (!(prot & IOPMP_MODE_NAPOT)) { // If not NAPOT, its in the other modes, configure with 2
		pmpaddr = (addr >> 2);
	} else {
        addrmask = (1UL << (log2len - 2)) - 1;
        pmpaddr	 = ((addr >> 2) & ~addrmask);
        pmpaddr |= (addrmask >> 1);
	}

    writel(prot, entry_config);
    writeq(pmpaddr, entry_addr);

	return 0;
}

void iopmp_srcmd_config(unsigned int n, u64 mds_bmap, u8 lock)
{
    u64 *srcmd_addr;
    u64 config;
    srcmd_addr = (void *)iopmp_inst_to_srcmd_en(iopmp, n);

    config  = readq(srcmd_addr);
    config |= (mds_bmap << 1) | (lock & 0x1);
    
    writeq(config, srcmd_addr);
}

void iopmp_mdcfg_config(unsigned int n, unsigned int t)
{
    u32 *mdcfg_addr;
    mdcfg_addr = (void *)iopmp_inst_to_mdcfg(iopmp, n);

    writel(t, mdcfg_addr);
}