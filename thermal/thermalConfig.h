/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_THERMAL_CONFIG_H__
#define THERMAL_THERMAL_CONFIG_H__

#include <vector>

#include "thermalData.h"
#include "thermalCommon.h"

namespace aidl::android::hardware::thermal::implementation {

class ThermalConfig {
public:
    ThermalConfig();
    ~ThermalConfig() = default;

    std::vector<struct target_therm_cfg> fetchConfig() {
        return thermalConfig;
    }

private:
    std::vector<struct target_therm_cfg> thermalConfig;
    int soc_id;
    ThermalCommon cmnInst;
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // THERMAL_THERMAL_CONFIG_H__
