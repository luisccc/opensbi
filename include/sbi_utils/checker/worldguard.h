
#ifndef __WORLDGUARD_H__
#define __WORLDGUARD_H__

#include <sbi/sbi_types.h>
#include <sbi/sbi_domain.h>

/** Representation of OpenSBI domain memory region */
struct worldguard_memregion {
	/** Node in linked list of mem regions */
	struct sbi_dlist node;

	/** Node in linked list of domains */
	struct sbi_memregion *region;

	/** Flags representing wid permitions */
    unsigned long perm;
};

/** Head of linked list of mem_regions */
extern struct sbi_dlist wg_memregion_list;

/** Iterate over each mem_region */
#define worldguard_mem_region_for_each(__mr) \
	sbi_list_for_each_entry(__mr, &wg_memregion_list, node)

int worldguard_init(void);
int worldguard_hart_init(unsigned long mlwid, unsigned long mwiddeleg);
int worldguard_memregion_init(unsigned long base,
				unsigned long order,
				unsigned long perm);

#endif
