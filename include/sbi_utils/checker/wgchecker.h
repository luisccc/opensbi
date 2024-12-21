/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __WORLDGUARD_CHECKER_H__
#define __WORLDGUARD_CHECKER_H__

#include <sbi/sbi_types.h>
#include <sbi_utils/checker/fdt_wgchecker.h>

int wgchecker_init(struct platform_wgchecker_data *wgchecker);

#endif
