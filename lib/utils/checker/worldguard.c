#include <sbi/sbi_error.h>
#include <sbi/sbi_heap.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi_utils/checker/worldguard.h>

SBI_LIST_HEAD(wg_memregion_list);

int worldguard_init(void)
{
    // For now just initialize the list
	SBI_INIT_LIST_HEAD(&wg_memregion_list);

    return 0;
}

int worldguard_hart_init(unsigned long mlwid, unsigned long mwiddeleg, unsigned long mwiddelegh)
{
    csr_write_num(CSR_MLWID, mlwid);
	csr_write_num(CSR_MWIDDELEG, mwiddeleg);

	if(mwiddelegh)
		csr_write_num(CSR_MWIDDELEGH2, mwiddelegh);

    return 0;
}

int worldguard_memregion_init(unsigned long base,
				unsigned long order,
				unsigned long perm)
{
	struct sbi_memregion *mem_reg = NULL;
    struct worldguard_memregion* reg;

    // Create Region
	reg = sbi_zalloc(sizeof(struct worldguard_memregion));

	if (!reg)
		return SBI_ENOMEM;

	if(sbi_memregion_init(base, order, &mem_reg) == SBI_OK){
		reg->region = mem_reg;
		reg->perm   = perm;

        // Add to the list
	    sbi_list_add_tail(&reg->node, &wg_memregion_list);
		return 0;
	}

	return SBI_ENOMEM;
}