/*
 * Copyright (C) 2022-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BacklightDevice.h"

#include <android-base/logging.h>

#include <algorithm>
#include <fstream>
#include <unistd.h>

#include "Utils.h"

#define LOG_TAG "BacklightDevice"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightBasePath = "/sys/class/backlight/";
static const uint32_t kDefaultMaxBrightness = 255;

static const std::string kBrightnessNode = "brightness";
static const std::string kMaxBrightnessNode = "max_brightness";

BacklightDevice::BacklightDevice(const std::string& name)
    : mName(name),
      mBasePath(kBacklightBasePath + name + "/"),
      mMaxBrightness(kDefaultMaxBrightness) {
    /*
     * Универсальный режим:
     * - читаем max_brightness из sysfs;
     * - не вводим device-specific hardcap вроде 3259;
     * - если ядро вернуло некорректное значение (0), откатываемся к 255.
     */
    uint32_t reportedMax = kDefaultMaxBrightness;
    readFromFile(mBasePath + kMaxBrightnessNode, reportedMax);

    if (reportedMax == 0) {
        LOG(WARNING) << "Invalid max_brightness=0 for " << mName
                     << ", fallback to default: " << kDefaultMaxBrightness;
        mMaxBrightness = kDefaultMaxBrightness;
    } else {
        mMaxBrightness = reportedMax;
    }

    LOG(INFO) << "Backlight " << mName << " max_brightness=" << mMaxBrightness;
}

std::string BacklightDevice::getName() const {
    return mName;
}

bool BacklightDevice::exists() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool BacklightDevice::setBrightness(uint8_t value) {
    const uint32_t scaled = std::min(scaleBrightness(value, mMaxBrightness), mMaxBrightness);
    return writeToFile(mBasePath + kBrightnessNode, scaled);
}

void BacklightDevice::dump(int fd) const {
    dprintf(fd, "Name: %s, exists: %d, base: %s, max: %u",
            mName.c_str(), exists(), mBasePath.c_str(), mMaxBrightness);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl