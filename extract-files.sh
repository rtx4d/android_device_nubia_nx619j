#!/bin/bash
#
# SPDX-FileCopyrightText: 2016 The CyanogenMod Project
# SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

function blob_fixup() {
    case "${1}" in
        vendor/lib/hw/audio.primary.sdm845.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libcutils.so" "libprocessgroup.so" "${2}"
            ;;
        vendor/lib64/hw/audio.primary.sdm845.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libcutils.so" "libprocessgroup.so" "${2}"
            ;;
        vendor/lib64/libgoodixhwfingerprint.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --remove-needed "libkeymaster_messages.so" "${2}"
            "${PATCHELF}" --remove-needed "libsoftkeymasterdevice.so" "${2}"
            ;;
        vendor/lib64/vendor.goodix.hardware.biometrics.fingerprint@2.1.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
            ;;
        *)
            return 1
            ;;
    esac

    return 0
}

# If we're being sourced by the common script that we called,
# stop right here. No need to go down the rabbit hole.
if [ "${BASH_SOURCE[0]}" != "${0}" ]; then
    return
fi

set -e

export DEVICE=nx619j
export DEVICE_COMMON=sdm845-common
export VENDOR=nubia
export VENDOR_COMMON=${VENDOR}

"./../../${VENDOR_COMMON}/${DEVICE_COMMON}/extract-files.sh" "$@"
