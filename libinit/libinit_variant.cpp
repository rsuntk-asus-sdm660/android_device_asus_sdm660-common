/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <libinit_utils.h>

#include <libinit_variant.h>

#include <chrono>
#include <fstream>
#include <thread>

#define SKU_PROP "ro.boot.product.hardware.sku"
#define NFC_HQ_PROP "ro.hq.support.nfc"
#define NFC_SUPPORT_PROP "persist.sys.nfc.supported"

void check_for_nfc()
{
    const std::string nfc_path = "/proc/NFC_CHECK";
    std::ifstream infile;
    std::string line;
    int retries = 30;

    while (retries-- > 0) {
        infile.open(nfc_path);
        if (infile.is_open())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    bool has_nfc = false;
    if (infile.is_open()) {
        if (std::getline(infile, line) && line.find("SUPPORTED") != std::string::npos) {
            has_nfc = true;
        }
        infile.close();
    }

    if (has_nfc) {
        property_override(NFC_HQ_PROP, "1");
        property_override(SKU_PROP, "NFC");
        property_override(NFC_SUPPORT_PROP, "true");
    } else {
        property_override(NFC_HQ_PROP, "0");
        property_override(SKU_PROP, "");
        property_override(NFC_SUPPORT_PROP, "false");
    }
}
