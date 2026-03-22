#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

import os
import sys

sys.argv = [sys.argv[0], '--regenerate_makefiles']

import importlib
extract_files = importlib.import_module('extract-files')

from extract_utils.main import ExtractUtils

COMMON_DEVICES = 'X00TD X01BD'
VENDOR_PATH = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    '..', '..', '..', 'vendor', 'asus', 'sdm660-common'
)

def add_device_filter():
    device_filter = f'ifeq ($(TARGET_DEVICE),$(filter $(TARGET_DEVICE),{COMMON_DEVICES}))'

    for mk in ['Android.mk', 'sdm660-common-vendor.mk']:
        mk_path = os.path.join(VENDOR_PATH, mk)
        if not os.path.exists(mk_path):
            continue

        with open(mk_path, 'r') as f:
            content = f.read()

        if 'filter $(TARGET_DEVICE)' in content:
            continue

        with open(mk_path, 'w') as f:
            f.write(f'{device_filter}\n')
            f.write(content)
            f.write('endif\n')

if __name__ == '__main__':
    utils = ExtractUtils.device(extract_files.module)
    utils.run()
    add_device_filter()
    print('Done! Device filter added for: ' + COMMON_DEVICES)
