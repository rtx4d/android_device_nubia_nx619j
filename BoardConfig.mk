#
# Copyright (C) 2018-2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

BUILD_BROKEN_ELF_PREBUILT_PRODUCT_COPY_FILES := true

# Inherit from nubia sdm845-common
include device/nubia/sdm845-common/BoardConfigCommon.mk

DEVICE_PATH := device/nubia/nx619j

# Releasetools
TARGET_BOARD_INFO_FILE := $(DEVICE_PATH)/board-info.txt

# Assert
TARGET_OTA_ASSERT_DEVICE := nx619j,NX619J

# Display
TARGET_SCREEN_DENSITY := 440

# HIDL
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/manifest.xml

# Kernel
TARGET_KERNEL_CONFIG += vendor/nubia/nx619j.config

# Partitions
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 4160749568
BOARD_USERDATAIMAGE_PARTITION_SIZE := 57089667072
BOARD_VENDORIMAGE_PARTITION_SIZE := 1073741824

# Properties
TARGET_VENDOR_PROP += $(DEVICE_PATH)/vendor.prop

# Recovery
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/rootdir/etc/fstab.qcom

# Sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor

# Inherit from the proprietary version
include vendor/nubia/nx619j/BoardConfigVendor.mk
