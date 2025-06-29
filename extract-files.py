#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2025 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'vendor/nubia/sdm845-common'
]

blob_fixups: blob_fixups_user_type = {
    ('vendor/lib/hw/audio.primary.sdm845.so', 'vendor/lib64/hw/audio.primary.sdm845.so'): blob_fixup()
        .replace_needed('libcutils.so', 'libprocessgroup.so'),
    'vendor/lib64/libgoodixhwfingerprint.so': blob_fixup()
        .remove_needed('libkeymaster_messages.so')
        .remove_needed('libsoftkeymasterdevice.so'),
    'vendor/lib64/vendor.goodix.hardware.biometrics.fingerprint@2.1.so': blob_fixup()
        .replace_needed('libhidlbase.so', 'libhidlbase-v32.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'nx619j',
    'nubia',
    namespace_imports=namespace_imports,
    blob_fixups=blob_fixups,
    check_elf=False,
)

if __name__ == '__main__':
    utils = ExtractUtils.device_with_common(module, 'sdm845-common', module.vendor)
    utils.run()
