/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_THERMAL_DATA_H__
#define THERMAL_THERMAL_DATA_H__

#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include <aidl/android/hardware/thermal/CoolingDevice.h>
#include <aidl/android/hardware/thermal/CoolingType.h>
#include <aidl/android/hardware/thermal/Temperature.h>
#include <aidl/android/hardware/thermal/TemperatureThreshold.h>
#include <aidl/android/hardware/thermal/TemperatureType.h>
#include <aidl/android/hardware/thermal/ThrottlingSeverity.h>

#define UNKNOWN_TEMPERATURE (NAN)

namespace aidl::android::hardware::thermal::implementation {

using ::aidl::android::hardware::thermal::CoolingDevice;
using ::aidl::android::hardware::thermal::CoolingType;
using ::aidl::android::hardware::thermal::Temperature;
using ::aidl::android::hardware::thermal::TemperatureThreshold;
using ::aidl::android::hardware::thermal::TemperatureType;
using ::aidl::android::hardware::thermal::ThrottlingSeverity;

static constexpr size_t kThrottlingSeverityCount =
    static_cast<size_t>(ThrottlingSeverity::SHUTDOWN) + 1;

struct target_therm_cfg {
    TemperatureType type;
    std::vector<std::string> sensor_list;
    std::string label;
    int throt_thresh;
    int shutdwn_thresh;
    bool positive_thresh_ramp;
};

struct therm_sensor {
    int tzn;
    int mulFactor;
    bool positiveThresh;
    std::string sensor_name;
    ThrottlingSeverity lastThrottleStatus;
    Temperature t;
    TemperatureThreshold thresh;
};

struct therm_cdev {
    int cdevn;
    CoolingDevice c;
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // THERMAL_THERMAL_DATA_H__
