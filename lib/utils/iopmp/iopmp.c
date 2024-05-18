#include <sbi_utils/iopmp/iopmp.h>

#include <sbi/sbi_console.h>
#include <sbi/riscv_io.h>

static struct iopmp_data *iopmp;

void iopmp_init_data(struct iopmp_data *iopmp_data)
{
    iopmp = iopmp_data;
    sbi_printf("Initing IOPMP\n");
}

void enable_iopmp(void)
{
    u32 config = 0x80000000; // Current spec the enable bit is the last bit of the HWCFG
    u32 *addr;

    addr = (void *)iopmp_inst_hwcfg0(iopmp);
    writel(config, addr);

    sbi_printf("Writing at %p \n", addr);
}

void entry_config(u64 addr, u8 mode, u8 access, u16 entry_num)
{
    sbi_printf("Does it error when configuring an entry? \n");
    u64 *entry_addr;
    u32 *entry_config;
    
    sbi_printf("MACRO gives: %lx \n", iopmp_inst_to_entry(iopmp, entry_num));
    entry_addr = (void *)iopmp_inst_to_entry(iopmp, entry_num);
    writeq(addr >> 2, entry_addr);

    sbi_printf("Writing at %p \n", entry_addr);

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
        mode = MODE_NA4;
    else
        mode = MODE_NAPOT;

    u64 addr = (base_addr + length / 2 - 1);

    entry_config(addr, mode, access, entry_num);
}

void set_entry_tor(u64 base_addr, u8 access, u16 entry_num)
{
    u8 mode;

    mode = MODE_TOR;

    entry_config(base_addr, mode, access, entry_num);
}

void set_entry_off(u64 base_addr, u8 access, u16 entry_num)
{
    u8 mode;

    mode = MODE_OFF;

    entry_config(base_addr, mode, access, entry_num);
}

void clean_all_entries()
{
    for (int i = 0; i < iopmp_inst_to_number_entries(iopmp); i++)
    {
        set_entry_off(0, MODE_OFF, i);
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