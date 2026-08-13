/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Migrated from HIDL 2.0 to AIDL V2.
 * Preserves all original logic: caps controller, uevent monitor, severity
 * estimation, threshold management, and callback notification.
 */

#include <algorithm>
#include <cerrno>
#include <ctype.h>
#include <string>

#include <android-base/logging.h>

#include "thermal.h"
#include "thermalMonitor.h"
#include "thermalCaps.h"

namespace aidl::android::hardware::thermal::implementation {

/*
 * Global caps controller — applies CPU/GPU/DDR frequency caps
 * based on temperature thresholds. Identical to HIDL version.
 */
static ThermalCapsController gCaps;

/*
 * Uevent → caps bridge.
 * Called by ThermalMonitor when a thermal uevent arrives.
 * Matches sensor name and feeds temperature to caps controller.
 * Identical logic to HIDL ApplyCapsFromUevent.
 */
static void ApplyCapsFromUevent(const std::string& sensor_name, int temp_mC) {
    std::string n = sensor_name;
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);

    if (n.find("cpuss-0-usr") != std::string::npos ||
        n.find("cpu-1-0-usr") != std::string::npos ||
        n.find("cpu") != std::string::npos ||
        n.find("soc") != std::string::npos) {
        gCaps.update((int64_t)temp_mC);
    }
}

/*
 * Constructor.
 * Initializes ThermalUtils with a callback that dispatches throttling
 * severity changes to registered AIDL callbacks.
 * Starts the uevent monitor thread exactly once (static lifetime).
 */
Thermal::Thermal()
    : utils(std::bind(&Thermal::sendThrottlingChangeCB, this,
                      std::placeholders::_1)) {
    static ThermalMonitor sMonitor(ApplyCapsFromUevent);
    static bool started = false;
    if (!started) {
        sMonitor.start();
        started = true;
        LOG(INFO) << "ThermalHAL: ThermalMonitor started (caps enabled)";
    }
}

/*
 * ════════════════════════════════════════════════════════════════
 * getTemperatures — return all sensor temperatures
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getTemperatures(
        std::vector<Temperature>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isSensorInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readTemperatures(false, TemperatureType::UNKNOWN,
                                *_aidl_return) <= 0) {
        LOG(ERROR) << "Sensor Temperature read failure.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * getTemperaturesWithType — return temperatures filtered by type
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getTemperaturesWithType(
        TemperatureType in_type,
        std::vector<Temperature>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isSensorInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readTemperatures(true, in_type, *_aidl_return) <= 0) {
        LOG(ERROR) << "Sensor Temperature read failure.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * getCoolingDevices — return all cooling device states
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getCoolingDevices(
        std::vector<CoolingDevice>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isCdevInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readCdevStates(false, CoolingType::CPU, *_aidl_return) <= 0) {
        LOG(ERROR) << "Failed to read thermal cooling devices.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * getCoolingDevicesWithType — return cooling devices filtered by type
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getCoolingDevicesWithType(
        CoolingType in_type,
        std::vector<CoolingDevice>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isCdevInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readCdevStates(true, in_type, *_aidl_return) <= 0) {
        LOG(ERROR) << "Failed to read thermal cooling devices.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * getTemperatureThresholds — return all sensor thresholds
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getTemperatureThresholds(
        std::vector<TemperatureThreshold>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isSensorInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readTemperatureThreshold(false, TemperatureType::UNKNOWN,
                                        *_aidl_return) <= 0) {
        LOG(ERROR) << "Sensor Threshold read failure or type not supported.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * getTemperatureThresholdsWithType — return thresholds filtered by type
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::getTemperatureThresholdsWithType(
        TemperatureType in_type,
        std::vector<TemperatureThreshold>* _aidl_return) {
    if (!_aidl_return) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    _aidl_return->clear();

    if (!utils.isSensorInitialized()) {
        LOG(ERROR) << "ThermalHAL not initialized properly.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    if (utils.readTemperatureThreshold(true, in_type, *_aidl_return) <= 0) {
        LOG(ERROR) << "Sensor Threshold read failure or type not supported.";
        return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * registerThermalChangedCallback — register callback for all types
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::registerThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& in_callback) {
    if (!in_callback) {
        LOG(ERROR) << "Invalid nullptr callback";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> _lock(thermal_cb_mutex);

    for (const auto& _cb : cb) {
        if (_cb.callback->asBinder() == in_callback->asBinder()) {
            LOG(ERROR) << "Same callback interface registered already";
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    }

    cb.push_back({in_callback, false, TemperatureType::UNKNOWN});
    LOG(DEBUG) << "A callback has been registered to ThermalHAL, isFilter: false";

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * registerThermalChangedCallbackWithType — register for specific type
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::registerThermalChangedCallbackWithType(
        const std::shared_ptr<IThermalChangedCallback>& in_callback,
        TemperatureType in_type) {
    if (!in_callback) {
        LOG(ERROR) << "Invalid nullptr callback";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    if (in_type == TemperatureType::BCL_VOLTAGE ||
        in_type == TemperatureType::BCL_CURRENT) {
        LOG(ERROR) << "BCL current and voltage notification not supported";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> _lock(thermal_cb_mutex);

    for (const auto& _cb : cb) {
        if (_cb.callback->asBinder() == in_callback->asBinder()) {
            LOG(ERROR) << "Same callback interface registered already";
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    }

    cb.push_back({in_callback, true, in_type});
    LOG(DEBUG) << "A callback has been registered to ThermalHAL, isFilter: true"
               << " Type: " << static_cast<int>(in_type);

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * unregisterThermalChangedCallback
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::unregisterThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& in_callback) {
    if (!in_callback) {
        LOG(ERROR) << "Invalid nullptr callback";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> _lock(thermal_cb_mutex);
    bool removed = false;

    cb.erase(
        std::remove_if(cb.begin(), cb.end(),
            [&in_callback, &removed](const CallbackSetting& cs) {
                if (cs.callback->asBinder() == in_callback->asBinder()) {
                    removed = true;
                    return true;
                }
                return false;
            }),
        cb.end());

    if (!removed) {
        LOG(ERROR) << "The callback was not registered before";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    LOG(DEBUG) << "Thermal callback unregistered";
    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * registerCoolingDeviceChangedCallbackWithType — AIDL V2
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::registerCoolingDeviceChangedCallbackWithType(
        const std::shared_ptr<ICoolingDeviceChangedCallback>& in_callback,
        CoolingType in_type) {
    if (!in_callback) {
        LOG(ERROR) << "Invalid nullptr cooling device callback";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> _lock(cdev_cb_mutex);

    for (const auto& _cb : cdev_cb) {
        if (_cb.callback->asBinder() == in_callback->asBinder()) {
            LOG(ERROR) << "Same cooling device callback registered already";
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
        }
    }

    cdev_cb.push_back({in_callback, true, in_type});
    LOG(DEBUG) << "Cooling device callback registered for type: "
               << static_cast<int>(in_type);

    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * unregisterCoolingDeviceChangedCallback — AIDL V2
 * ════════════════════════════════════════════════════════════════
 */
ndk::ScopedAStatus Thermal::unregisterCoolingDeviceChangedCallback(
        const std::shared_ptr<ICoolingDeviceChangedCallback>& in_callback) {
    if (!in_callback) {
        LOG(ERROR) << "Invalid nullptr cooling device callback";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    std::lock_guard<std::mutex> _lock(cdev_cb_mutex);
    bool removed = false;

    cdev_cb.erase(
        std::remove_if(cdev_cb.begin(), cdev_cb.end(),
            [&in_callback, &removed](const CdevCallbackSetting& cs) {
                if (cs.callback->asBinder() == in_callback->asBinder()) {
                    removed = true;
                    return true;
                }
                return false;
            }),
        cdev_cb.end());

    if (!removed) {
        LOG(ERROR) << "Cooling device callback was not registered";
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    LOG(DEBUG) << "Cooling device callback unregistered";
    return ndk::ScopedAStatus::ok();
}

/*
 * ════════════════════════════════════════════════════════════════
 * sendThrottlingChangeCB — internal, called from ThermalUtils::Notify
 * Dispatches throttling severity changes to all registered callbacks.
 * If a callback fails, it is automatically removed (same as HIDL).
 * ════════════════════════════════════════════════════════════════
 */
void Thermal::sendThrottlingChangeCB(const Temperature &t) {
    std::lock_guard<std::mutex> _lock(thermal_cb_mutex);

    LOG(DEBUG) << "Throttle Severity change:"
               << " Type: " << static_cast<int>(t.type)
               << " Name: " << t.name
               << " Value: " << t.value
               << " ThrottlingStatus: " << static_cast<int>(t.throttlingStatus);

    auto it = cb.begin();
    while (it != cb.end()) {
        if (!it->is_filter_type || it->type == t.type) {
            auto ret = it->callback->notifyThrottling(t);
            if (!ret.isOk()) {
                LOG(ERROR) << "Notify callback execution error. Removing";
                it = cb.erase(it);
                continue;
            }
        }
        it++;
    }
}

}  // namespace aidl::android::hardware::thermal::implementation
