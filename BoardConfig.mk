#
# Copyright (C) 2018-2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from nubia sdm845-common
-include device/nubia/sdm845-common/BoardConfigCommon.mk

DEVICE_PATH := device/nubia/nx619j

# Assert
TARGET_OTA_ASSERT_DEVICE := nx619j,NX619J

# Kernel
TARGET_KERNEL_CONFIG := lineageos_nx619j_defconfig

# Inherit from the proprietary version
-include vendor/nubia/nx619j/BoardConfigVendor.mk
