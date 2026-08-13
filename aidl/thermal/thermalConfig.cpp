/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Updated for ASUS X00TD/X01BD - kernel 4.19
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdlib>
#include <unordered_map>

#include <android-base/logging.h>

#include "thermalConfig.h"
#include "thermalData.h"

namespace aidl::android::hardware::thermal::implementation {

constexpr std::string_view socIDPath("/sys/devices/soc0/soc_id");

/*
 * Thermal Zone Map for X00TD/X01BD (kernel 4.19):
 * ══════════════════════════════════════════════════════════════════
 * tz0:  pm660-tz       - PMIC temperature (mC)
 * tz1:  pm660l-tz      - PMIC-L temperature (mC)
 * tz2:  xo-therm-adc   - Crystal oscillator thermistor (mC)
 * tz3:  msm-therm-adc  - MSM thermistor (mC)
 * tz4:  ibat-high      - Battery current high (mA)
 * tz5:  ibat-vhigh     - Battery current very high (mA)
 * tz6:  vbat_adc       - Battery voltage (mV)
 * tz7:  vbat_low       - Battery voltage low (mV)
 * tz8:  vbat_too_low   - Battery voltage critical (mV)
 * tz9:  mpm-usr        - Modem power manager (mC)
 * tz10: cpuss-0-usr    - CPU Silver cluster 0 (mC)
 * tz11: cpuss-1-usr    - CPU Silver cluster 1 (mC)
 * tz12: cpu-1-0-usr    - CPU Gold core 4 (mC)
 * tz13: cpu-1-1-usr    - CPU Gold core 5 (mC)
 * tz14: cpu-1-2-usr    - CPU Gold core 6 (mC)
 * tz15: cpu-1-3-usr    - CPU Gold core 7 (mC)
 * tz16: cpuss-2-usr    - CPU Silver cluster 2 (mC)
 * tz17: gpu-usr        - GPU temperature (mC)
 * tz18: video-usr      - Video codec (mC)
 * tz19: mdm-core-usr   - Modem core (mC)
 * tz20: camera-usr     - Camera ISP (mC)
 * tz21: cpuss-3-usr    - CPU Silver cluster 3 (mC)
 * tz22-27: *-step      - Step-wise cooling (kernel internal)
 * tz28: soc            - Battery SoC % (raw percentage)
 * tz29: battery        - Battery temperature (mC)
 * tz30: bms            - BMS temperature (mC)
 * ══════════════════════════════════════════════════════════════════
 */

/*
 * CPU sensor mapping - using all available sensors
 * Silver cluster (cores 0-3): cpuss-0/1/2/3-usr
 * Gold cluster (cores 4-7): cpu-1-0/1/2/3-usr
 */
std::vector<std::string> cpu_sensors_unified = {
    "cpuss-0-usr",   // Core 0 (Silver)
    "cpuss-1-usr",   // Core 1 (Silver)
    "cpuss-2-usr",   // Core 2 (Silver)
    "cpuss-3-usr",   // Core 3 (Silver)
    "cpu-1-0-usr",   // Core 4 (Gold)
    "cpu-1-1-usr",   // Core 5 (Gold)
    "cpu-1-2-usr",   // Core 6 (Gold)
    "cpu-1-3-usr",   // Core 7 (Gold)
};

std::vector<struct target_therm_cfg> sensor_cfg_unified = {
    {
        TemperatureType::CPU,
        cpu_sensors_unified,
        "",
        95000,        // Throttling threshold (mC)
        115000,       // Shutdown threshold (mC)
        true,         // Positive ramp
    },
    {
        TemperatureType::GPU,
        {"gpu-usr"},
        "GPU",
        95000,
        115000,
        true,
    },
    {
        TemperatureType::SKIN,
        {"msm-therm-adc"},
        "skin",
        45000,
        65000,
        true,
    },
    {
        /*
         * Battery temperature (tz29)
         * Reports in milliCelsius (31000 = 31.0°C)
         */
        TemperatureType::BATTERY,
        {"battery"},
        "battery",
        45000,        // 45°C - throttle charging
        60000,        // 60°C - critical
        true,
    },
    {
        /*
         * BMS temperature (tz30)
         * Battery Management System sensor
         * Reports in milliCelsius (31000 = 31.0°C)
         */
        TemperatureType::BATTERY,
        {"bms"},
        "bms",
        46000,        // Slightly offset to avoid duplicate alerts
        60000,
        true,
    },
    {
        /*
         * BCL Current monitoring (tz4)
         * ibat-high reports battery discharge current
         */
        TemperatureType::BCL_CURRENT,
        {"ibat-high"},
        "ibat",
        4500,         // 4.5A
        5000,         // 5A critical
        true,
    },
    {
        /*
         * BCL Voltage monitoring (tz6)
         * vbat_adc reports battery voltage in mV
         */
        TemperatureType::BCL_VOLTAGE,
        {"vbat_adc"},
        "vbat",
        3200,         // 3.2V low
        3000,         // 3.0V critical
        false,        // Negative ramp (lower = worse)
    },
    {
        /*
         * Battery SoC percentage (tz28)
         * Reports raw percentage (100 = 100%)
         */
        TemperatureType::BCL_PERCENTAGE,
        {"soc"},
        "soc",
        10,           // 10% - start power saving
        2,            // 2% - critical
        false,        // Negative ramp
    },
};

std::vector<struct target_therm_cfg> bcl_conf = {};

const std::unordered_map<int, std::vector<struct target_therm_cfg>>
    msm_soc_map = {
    {345, sensor_cfg_unified},  // SDM636 (X00TD)
    {317, sensor_cfg_unified},  // SDM660 (X01BD)
    {324, sensor_cfg_unified},  // SDM660 variant
    {326, sensor_cfg_unified},  // SDM660 variant
};

ThermalConfig::ThermalConfig() : cmnInst() {
    std::unordered_map<int, std::vector<struct target_therm_cfg>>::const_iterator it;
    std::string soc_val;

    if (cmnInst.readFromFile(socIDPath, soc_val) <= 0) {
        LOG(ERROR) << "ThermalHAL: soc ID fetch error";
        soc_id = 317;
    } else {
        soc_id = std::atoi(soc_val.c_str());
    }

    if (soc_id <= 0) soc_id = 317;

    it = msm_soc_map.find(soc_id);
    if (it == msm_soc_map.end()) {
        thermalConfig = sensor_cfg_unified;
    } else {
        thermalConfig = it->second;
    }

    LOG(INFO) << "ThermalHAL: SoC ID=" << soc_id
              << " sensors=" << thermalConfig.size();
}

}  // namespace aidl::android::hardware::thermal::implementation
