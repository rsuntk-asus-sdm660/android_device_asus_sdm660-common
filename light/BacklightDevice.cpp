/*
 * Copyright (C) 2022-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BacklightDevice.h"

#include <android-base/logging.h>
#include <fstream>
#include <unistd.h>
#include <algorithm> // для std::min
#include "Utils.h"

#define LOG_TAG "BacklightDevice"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightBasePath = "/sys/class/backlight/";
static const uint32_t kDefaultMaxBrightness = 255;

// ★ REAL HARDWARE LIMIT FOR X00TD/X01BD
// Ядро врет про 4095. Реальный лимит PM660L WLED = 3259.
static const uint32_t kTrueHardwareMax = 3259;

static const std::string kBrightnessNode = "brightness";
static const std::string kMaxBrightnessNode = "max_brightness";

BacklightDevice::BacklightDevice(const std::string& name)
    : mName(name),
      mBasePath(kBacklightBasePath + name + "/"),
      mMaxBrightness(kDefaultMaxBrightness) {
    
    // Читаем то, что дает ядро (4095)
    readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness);

    // ★ FIX: Если ядро дает больше реального лимита, обрезаем
    if (mMaxBrightness > kTrueHardwareMax) {
        LOG(WARNING) << "Kernel reported max brightness " << mMaxBrightness 
                     << " is too high! Capping to hardware safe limit: " << kTrueHardwareMax;
        mMaxBrightness = kTrueHardwareMax;
    }
}

std::string BacklightDevice::getName() const {
    return mName;
}

bool BacklightDevice::exists() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool BacklightDevice::setBrightness(uint8_t value) {
    // Теперь scaleBrightness будет масштабировать 0-255 в 0-3259
    // OVP не сработает!
    return writeToFile(mBasePath + kBrightnessNode, scaleBrightness(value, mMaxBrightness));
}

void BacklightDevice::dump(int fd) const {
    dprintf(fd, "Name: %s, exists: %d, base: %s, max: %u",
            mName.c_str(), exists(), mBasePath.c_str(), mMaxBrightness);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
