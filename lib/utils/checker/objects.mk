#
# SPDX-License-Identifier: BSD-2-Clause
#

libsbiutils-objs-$(CONFIG_FDT_WORLDGUARD) += checker/fdt_worldguard.o
libsbiutils-objs-$(CONFIG_FDT_WORLDGUARD) += checker/worldguard.o
libsbiutils-objs-$(CONFIG_FDT_WORLDGUARD) += checker/fdt_wgchecker.o
libsbiutils-objs-$(CONFIG_FDT_WORLDGUARD) += checker/wgchecker.o
