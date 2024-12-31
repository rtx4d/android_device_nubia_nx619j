#
# Copyright (C) 2018-2024 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from nx619j device
$(call inherit-product, device/nubia/nx619j/device.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Device identifier. This must come after all inclusions.
PRODUCT_NAME := lineage_nx619j
PRODUCT_DEVICE := nx619j
PRODUCT_MANUFACTURER := Nubia
PRODUCT_BRAND := Nubia
PRODUCT_MODEL := Red Magic Mars

PRODUCT_GMS_CLIENTID_BASE := android-nubia

TARGET_VENDOR_PRODUCT_NAME := NX619J
TARGET_VENDOR_DEVICE_NAME := NX619J

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="NX619J-user 9 PKQ1.180929.001 eng.nubia.20201203.235401 release-keys" \
    BuildFingerprint=nubia/NX619J/NX619J:9/PKQ1.180929.001/eng.nubia.20201203.235401:user/release-keys
