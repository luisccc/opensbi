#include <sbi_utils/iopmp/iopmp.h>

#include <sbi/sbi_domain.h>
#include <sbi/sbi_console.h>
#include <sbi/riscv_io.h>

static struct iopmp_data *iopmp;

void iopmp_init_data(struct iopmp_data *iopmp_data)
{
    iopmp = iopmp_data;
    sbi_printf("Initing IOPMP\n");
}

void iopmp_configure(void)
{
    u32 i;
    u32* rrid;
    u64  bmap = 0;
    struct sbi_domain_memregion *reg;
	struct sbi_iodomain *dom;
    unsigned int entry_flags = 0, entry_count = 0, rrid_count = 0;//, md_idx;
	//unsigned int entry_count = iopmp->number_entries, md_count = iopmp->number_mds;

    sbi_printf("IOPMP \n");
	sbi_iodomain_for_each(i, dom) {
        sbi_printf("Domain %d \n", i);
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

            iopmp_entry_set(entry_count++, entry_flags, reg->base, reg->order);
    
            sbi_printf("entry_flags: %x base_addr: %lx, order: %lx \n", entry_flags, reg->base, reg->order);
        }

        // Setup domains MDCFG
        iopmp_mdcfg_config(i, entry_count);
        sbi_printf("entry_count: %d\n", entry_count);

        // Setup RRID MDCFG
        sbi_iodomain_for_each_rrid(dom, rrid, rrid_count){
            bmap = (1 << i);    // Create a bitmap value with the current value of md
            sbi_printf("Pushing bmap: %lx to rrid %d \n", bmap, *rrid);
            iopmp_srcmd_config(*rrid, bmap, 0);
        }

	}
}

void enable_iopmp(void)
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

    sbi_printf("Actual flags: %lx \n", prot);

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

void entry_config(u64 addr, u8 mode, u8 access, u16 entry_num)
{
    u64 *entry_addr;
    u32 *entry_config;
    
    entry_addr = (void *)iopmp_inst_to_entry(iopmp, entry_num);
    writeq(addr >> 2, entry_addr);

    u64 value = readq(entry_addr);

    if (value != addr >> 2)
        sbi_printf("IOPMP: Failed in configuring entry %d. Supposed %lx, actual %lx \n", entry_num, addr >> 2, value);

    entry_config = (void *)iopmp_inst_to_entry_cfg(iopmp, entry_num);
    u32 config = ((mode & 0x3) << 3) + (access & 0x7);
    writel(config, entry_config);
}

// TODO: Check if length is a multiple of the base address
void set_entry_napot(u64 base_addr, u64 length, u8 access, u16 entry_num)
{
    u8 mode;

    if (length < 8)
        mode = IOPMP_MODE_NA4;
    else
        mode = IOPMP_MODE_NAPOT;

    u64 addr = (base_addr + length / 2 - 1);

    entry_config(addr, mode, access, entry_num);
}

void set_entry_tor(u64 base_addr, u8 access, u16 entry_num)
{
    u8 mode;

    mode = IOPMP_MODE_TOR;

    entry_config(base_addr, mode, access, entry_num);
}

void set_entry_off(u64 base_addr, u8 access, u16 entry_num)
{
    u8 mode;

    mode = IOPMP_MODE_OFF;

    entry_config(base_addr, mode, access, entry_num);
}

void clean_all_entries()
{
    for (int i = 0; i < iopmp_inst_to_number_entries(iopmp); i++)
    {
        set_entry_off(0, IOPMP_MODE_OFF, i);
    }
}

void clean_error_reg()
{
    u32 *addr;
    addr = (void *)iopmp_inst_reqinfo(iopmp);
    u32 config = 0x1; // Current spec the ip bit is the first bit of the err_reqinfo

    writel(config, addr);
}

u32 read_error_reqinfo()
{
    u32 *addr;
    addr = (void *)iopmp_inst_reqinfo(iopmp);
    u32 result = readl(addr);

    return result;
}

u32 read_error_reqid()
{
    u32 *addr;
    addr = (void *)iopmp_inst_reqid(iopmp);
    u32 result = readl(addr);

    return result;
}

u64 read_error_reqaddr()
{
    u64 *addr;
    addr = (void *)iopmp_inst_reqaddr(iopmp);
    u64 result = readq(addr);

    return result;
}

void srcmd_entry_config(u16 *mds, u8 number_mds, u8 lock, u8 entry_num)
{
    u64 *entry_addr;
    entry_addr = (void *)iopmp_inst_to_srcmd_en(iopmp, entry_num);

    u64 md_value = 0;
    for (int i = 0; i < number_mds; i++)
    {
        md_value = md_value | (1 << mds[i]);
    }

    u64 config = (md_value << 1) + (lock & 0x1);
    writeq(config, entry_addr);
}

void mdcfg_entry_config(u16 t, u8 entry_num)
{
    u32 *entry_addr;
    entry_addr = (void *)iopmp_inst_to_mdcfg(iopmp, entry_num);

    writel((u32)t, entry_addr);
}