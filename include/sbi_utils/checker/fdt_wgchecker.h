/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __FDT_WGCHECKER_H__
#define __FDT_WGCHECKER_H__

#include <sbi/sbi_types.h>

struct platform_wgchecker_data {
	unsigned long addr;
};

int fdt_wgchecker_init(const void *fdt, int nodeoffset);

int fdt_parse_wgchecker_node(const void *fdt, int nodeoffset,
				      struct platform_wgchecker_data *wgchecker);

#endif
