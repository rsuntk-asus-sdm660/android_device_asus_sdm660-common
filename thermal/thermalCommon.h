/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_THERMAL_COMMON_H__
#define THERMAL_THERMAL_COMMON_H__

#include <string>
#include <vector>

#include "thermalData.h"

namespace aidl::android::hardware::thermal::implementation {

class ThermalCommon {
public:
    ThermalCommon();
    ~ThermalCommon() = default;

    int readFromFile(std::string_view path, std::string& out);
    int initThermalZones(std::vector<struct target_therm_cfg>& cfg);
    void initThreshold(struct therm_sensor& sens);
    int initCdev();

    int read_cdev_state(struct therm_cdev& cdev);
    int read_temperature(struct therm_sensor& sensor);
    int estimateSeverity(struct therm_sensor& sensor);

    std::vector<struct therm_sensor> fetch_sensor_list() {
        return sens;
    }
    std::vector<struct therm_cdev> fetch_cdev_list() {
        return cdev;
    }

private:
    int ncpus;
    std::vector<struct target_therm_cfg> cfg;
    std::vector<struct therm_sensor> sens;
    std::vector<struct therm_cdev> cdev;

    int initializeCpuSensor(struct target_therm_cfg& cpu_cfg);
    int initialize_sensor(struct target_therm_cfg& cfg, int sens_idx);
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // THERMAL_THERMAL_COMMON_H__
