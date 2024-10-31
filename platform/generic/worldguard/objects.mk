#
# SPDX-License-Identifier: BSD-2-Clause
#

carray-platform_override_modules-$(CONFIG_PLATFORM_WORLDGUARD) += worldguard_generic
platform-objs-$(CONFIG_PLATFORM_WORLDGUARD) += worldguard/worldguard_generic.o
