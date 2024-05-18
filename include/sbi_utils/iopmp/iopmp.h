#ifndef _IOPMP_H_
#define _IOPMP_H_

#include <sbi/sbi_types.h>

struct iopmp_data {
	/* Public details */
	unsigned long addr;
	unsigned long size;

    u16 number_rrids;
    u16 number_entries;
    u8  number_mds;
};

#define IOPMP_SIZE		0x4000

#define MODE_OFF        (0ULL)
#define MODE_TOR        (1ULL)
#define MODE_NA4        (2ULL)
#define MODE_NAPOT      (3ULL)

#define ACCESS_NONE     (0ULL)
#define ACCESS_READ     (1ULL)
#define ACCESS_WRITE    (2ULL)
#define ACCESS_EXEC     (4ULL)

#define NO_ERROR            (0ULL)
#define READ_ERROR          (1ULL)
#define WRITE_ERROR         (2ULL)
#define EXECUTION_ERROR     (3ULL)
#define PARTIAL_ERROR       (4ULL)
#define NOT_HIT_ERROR       (5ULL)
#define UNKOWN_SID_ERROR    (6ULL)

#define IOPMP_VERSION_OFFSET        0x0
#define IOPMP_IMP_OFFSET            0x4
#define IOPMP_HWCFG0_OFFSET         0x8
#define IOPMP_HWCFG1_OFFSET         0xc
#define IOPMP_HWCFG2_OFFSET         0x10
#define IOPMP_ENTRY_OFFSET_OFFSET   0x14
#define IOPMP_ERRREACT_OFFSET       0x18
#define IOPMP_MDCFGLCK_OFFSET       0x48
#define IOPMP_ENTRYLCK_OFFSET       0x4c
#define IOPMP_ERR_REQINFO_OFFSET    0x60
#define IOPMP_ERR_REQID_OFFSET      0x64
#define IOPMP_ERR_REQADDR_OFFSET    0x68
#define IOPMP_ERR_REQADDRH_OFFSET   0x6c
#define IOPMP_MDCFG_OFFSET          0x800

#define IOPMP_SRCMD_OFFSET          0x1000
#define IOPMP_SRCMD_EN_OFFSET       0x0
#define IOPMP_SRCMD_ENH_OFFSET      0x4

#define IOPMP_ENTRY_ARRAY_OFFSET    0x2000
#define IOPMP_ENTRY_ADDR_OFFSET     0x0
#define IOPMP_ENTRY_ADDRH_OFFSET    0x4
#define IOPMP_ENTRY_CFG_OFFSET      0x8

#define IOPMP_REG_ADDR(OFF)     (IOPMP_BASE_ADDR + OFF)

#define REQ_INFO_IP_MASK(value)     (value & 0x1)
#define REQ_INFO_TTYPE_MASK(value)  ((value >> 1) & 0x3)
#define REQ_INFO_ETYPE_MASK(value)  ((value >> 4) & 0x7)

#define REQ_ID_SID(value)           (value & 0xffff)
#define REQ_ID_EID(value)           ((value >> 16) & 0xffff)



#define iopmp_inst_to_base_addr(__inst) \
    __inst->addr

#define iopmp_inst_to_number_entries(__inst) \
    __inst->number_entries

#define iopmp_inst_hwcfg0(__inst) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_HWCFG0_OFFSET

#define iopmp_inst_reqinfo(__inst) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_ERR_REQINFO_OFFSET

#define iopmp_inst_reqid(__inst) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_ERR_REQID_OFFSET

#define iopmp_inst_reqaddr(__inst) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_ERR_REQADDR_OFFSET

#define iopmp_inst_reqaddrh(__inst) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_ERR_REQADDRH_OFFSET

#define iopmp_inst_to_mdcfg(__inst, __num) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_MDCFG_OFFSET + (__num * 4)

#define iopmp_inst_to_srcmd(__inst, __num) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_SRCMD_OFFSET + (__num * 32)

#define iopmp_inst_to_srcmd_en(__inst, __num) \
    iopmp_inst_to_srcmd(__inst, __num) + IOPMP_SRCMD_EN_OFFSET

#define iopmp_inst_to_srcmd_enh(__inst, __num) \
    iopmp_inst_to_srcmd(__inst, __num) + IOPMP_SRCMD_ENH_OFFSET

#define iopmp_inst_to_entry(__inst, __num) \
    iopmp_inst_to_base_addr(__inst) + IOPMP_ENTRY_ARRAY_OFFSET + (__num * 16)

#define iopmp_inst_to_entry_addr(__inst, __num) \
    iopmp_inst_to_entry(__inst, __num) + IOPMP_ENTRY_ADDR_OFFSET

#define iopmp_inst_to_entry_addrh(__inst, __num) \
    iopmp_inst_to_entry(__inst, __num) + IOPMP_ENTRY_ADDRH_OFFSET

#define iopmp_inst_to_entry_cfg(__inst, __num) \
    iopmp_inst_to_entry(__inst, __num) + IOPMP_ENTRY_CFG_OFFSET



void iopmp_init_data(struct iopmp_data *iopmp);
void enable_iopmp(void);
void entry_config(u64 addr, u8 mode, u8 access, u16 entry_num);
void set_entry_napot(u64 base_addr, u64 length, u8 access, u16 entry_num);
void set_entry_tor(u64 base_addr, u8 access, u16 entry_num);
void set_entry_off(u64 base_addr, u8 access, u16 entry_num);
void srcmd_entry_config(u16* mds, u8 number_mds, u8 lock, u8 entry_num);
void mdcfg_entry_config(u16 t, u8 entry_num);
void clean_all_entries();

void clean_error_reg();
u32 read_error_reqinfo();
u32 read_error_reqid();
u64 read_error_reqaddr();

#endif  /* _RV_IOPMP_H_ */