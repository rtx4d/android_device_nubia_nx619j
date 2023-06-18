#
# Copyright (C) 2018-2023 The LineageOS Project
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

PRODUCT_BUILD_PROP_OVERRIDES += TARGET_DEVICE=NX619J PRODUCT_NAME=NX619J PRODUCT_MODEL=NX619J PRODUCT_DEVICE=NX619J

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="NX619J-user 9 PKQ1.180929.001 eng.nubia.20201203.235401 release-keys"

BUILD_FINGERPRINT := nubia/NX619J/NX619J:9/PKQ1.180929.001/eng.nubia.20201203.235401:user/release-keys
