/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_THERMAL_UTILS_H__
#define THERMAL_THERMAL_UTILS_H__

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "thermalConfig.h"
#include "thermalMonitor.h"
#include "thermalCommon.h"
#include "thermalData.h"

namespace aidl::android::hardware::thermal::implementation {

using ueventCB = std::function<void(const Temperature &t)>;

class ThermalUtils {
public:
    ThermalUtils(const ueventCB &inp_cb);
    ~ThermalUtils() = default;

    bool isSensorInitialized() { return is_sensor_init; }
    bool isCdevInitialized() { return is_cdev_init; }

    int readTemperatures(bool filterType, TemperatureType type,
                         std::vector<Temperature>& temperatures);
    int readTemperatureThreshold(bool filterType, TemperatureType type,
                                 std::vector<TemperatureThreshold>& thresh);
    int readCdevStates(bool filterType, CoolingType type,
                       std::vector<CoolingDevice>& cdev);

private:
    bool is_sensor_init;
    bool is_cdev_init;
    ThermalConfig cfg;
    ThermalCommon cmnInst;
    ThermalMonitor monitor;
    std::unordered_map<std::string, struct therm_sensor> thermalConfig;
    std::vector<struct therm_cdev> cdevList;
    std::mutex sens_cb_mutex;
    ueventCB cb;

    void ueventParse(std::string sensor_name, int temp);
    void Notify(struct therm_sensor& sens);
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // THERMAL_THERMAL_UTILS_H__
