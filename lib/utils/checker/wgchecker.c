#include <sbi/riscv_asm.h>
#include <sbi/riscv_io.h>
#include <sbi_utils/checker/worldguard.h>
#include <sbi_utils/checker/wgchecker.h>

// typedef struct
// {
// 	u64 addr;
// 	u64 perm;
// 	u32 cfg;
// 	// Reserved fields, helps maintain memory alignment
// 	u32 rsv0;
// 	u64 rsv1;
// } __attribute__((aligned(32), __packed__)) slot_t;

typedef struct
{
	u64 addr;
	u64 perm;
	u32 cfg;
	// Reserved fields, helps maintain memory alignment
	u32 rsv;
	u64 permh;
	u64 permh2;
	u64 permh3;
	u64 rsv1;
	u64 rsv2;
} __attribute__((aligned(64), __packed__)) slot_t;

typedef struct
{
	u32 vendor;
	u32 impid;
	u32 nslots;
	// Reserved fields, helps maintain memory alignment
	u32 rsv0;
	u64 errcause;
	u64 erraddr;
} checker_t;

static volatile checker_t 	*checker;
static volatile slot_t 	  	*slots;

static u64 encode_addr(unsigned long addr, unsigned long log2len, unsigned long enconding)
{
	unsigned long addrmask, encoded_addr;

	/* encode address */
	if (!(enconding & 0x2)) { // If not NAPOT, its in the other modes, configure with 2
		encoded_addr  = (addr >> 2);
	} else {
        addrmask 	  = (1UL << (log2len - 2)) - 1;
        encoded_addr  = ((addr >> 2) & ~addrmask);
        encoded_addr |= (addrmask >> 1);
	}

	return encoded_addr;
}

int wgchecker_init(struct platform_wgchecker_data *wgchecker){
	u32 num_slots;
	u64 checker_start, checker_end;

	checker = (volatile checker_t *)wgchecker->addr;
	slots = (slot_t *)(wgchecker->addr + 0x20);

	num_slots = checker->nslots;
	if (num_slots == 0)
		return 0;
	
	checker_start = (slots[0].addr << 2);
	checker_end   = (slots[num_slots].addr << 2);

	const struct worldguard_memregion *mem_reg;
	unsigned long rend;
	// Special case for single slot
	// Check if any mem region occupies the entire slot's region
	// As per the spec: The last slot can only be configured to OFF or TOR.
	// If no region occupies the entire slot, then it is OFF
	if (num_slots == 1)
	{
		worldguard_mem_region_for_each(mem_reg){
			rend = (mem_reg->region->order < __riscv_xlen) ?
			mem_reg->region->base + ((1UL << mem_reg->region->order) - 1) : -1UL;

			if(mem_reg->region->base == checker_start && rend == checker_end - 1){
				slots[1].perm = mem_reg->perm; // Give permissions depending on wid
				slots[1].permh  = mem_reg->permh; // Give permissions depending on wid
				slots[1].permh2 = mem_reg->permh2; // Give permissions depending on wid
				slots[1].permh3 = mem_reg->permh3; // Give permissions depending on wid
				slots[1].cfg  = 0x1; // Set to TOR
			}
		}
	}
	else
	{	
		u32 slot = 1;
		
		worldguard_mem_region_for_each(mem_reg){
			rend = (mem_reg->region->order < __riscv_xlen) ?
			mem_reg->region->base + ((1UL << mem_reg->region->order) - 1) : -1UL;

			if(mem_reg->region->base >= checker_start && rend < checker_end){
				slots[slot].perm = mem_reg->perm; // Give permissions depending on wid
				slots[slot].permh  = mem_reg->permh; // Give permissions depending on wid
				slots[slot].permh2 = mem_reg->permh2; // Give permissions depending on wid
				slots[slot].permh3 = mem_reg->permh3; // Give permissions depending on wid
				slots[slot].cfg  = 0x3;    // Set to NAPOT
				slots[slot].addr = encode_addr(mem_reg->region->base, mem_reg->region->order, 0x3);

				slot++; // For now each mem_region corresponds to 1 slot
			}
		}
	}
	
	return 0;
}
