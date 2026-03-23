/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Migrated from HIDL 2.0 to AIDL V2.
 */

#ifndef ANDROID_QTI_VENDOR_THERMAL_H
#define ANDROID_QTI_VENDOR_THERMAL_H

#include <mutex>
#include <vector>

#include <aidl/android/hardware/thermal/BnThermal.h>
#include <aidl/android/hardware/thermal/ICoolingDeviceChangedCallback.h>
#include <aidl/android/hardware/thermal/IThermalChangedCallback.h>

#include "thermalUtils.h"
#include "thermalData.h"

namespace aidl::android::hardware::thermal::implementation {

using ::aidl::android::hardware::thermal::ICoolingDeviceChangedCallback;
using ::aidl::android::hardware::thermal::IThermalChangedCallback;

struct CallbackSetting {
    std::shared_ptr<IThermalChangedCallback> callback;
    bool is_filter_type;
    TemperatureType type;
};

struct CdevCallbackSetting {
    std::shared_ptr<ICoolingDeviceChangedCallback> callback;
    bool is_filter_type;
    CoolingType type;
};

class Thermal : public BnThermal {
public:
    Thermal();
    ~Thermal() override = default;

    Thermal(const Thermal &) = delete;
    void operator=(const Thermal &) = delete;

    ndk::ScopedAStatus getCoolingDevices(
        std::vector<CoolingDevice>* _aidl_return) override;

    ndk::ScopedAStatus getCoolingDevicesWithType(
        CoolingType in_type,
        std::vector<CoolingDevice>* _aidl_return) override;

    ndk::ScopedAStatus getTemperatures(
        std::vector<Temperature>* _aidl_return) override;

    ndk::ScopedAStatus getTemperaturesWithType(
        TemperatureType in_type,
        std::vector<Temperature>* _aidl_return) override;

    ndk::ScopedAStatus getTemperatureThresholds(
        std::vector<TemperatureThreshold>* _aidl_return) override;

    ndk::ScopedAStatus getTemperatureThresholdsWithType(
        TemperatureType in_type,
        std::vector<TemperatureThreshold>* _aidl_return) override;

    ndk::ScopedAStatus registerThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& in_callback) override;

    ndk::ScopedAStatus registerThermalChangedCallbackWithType(
        const std::shared_ptr<IThermalChangedCallback>& in_callback,
        TemperatureType in_type) override;

    ndk::ScopedAStatus unregisterThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& in_callback) override;

    ndk::ScopedAStatus registerCoolingDeviceChangedCallbackWithType(
        const std::shared_ptr<ICoolingDeviceChangedCallback>& in_callback,
        CoolingType in_type) override;

    ndk::ScopedAStatus unregisterCoolingDeviceChangedCallback(
        const std::shared_ptr<ICoolingDeviceChangedCallback>& in_callback) override;

    void sendThrottlingChangeCB(const Temperature &t);

private:
    std::mutex thermal_cb_mutex;
    std::vector<CallbackSetting> cb;

    std::mutex cdev_cb_mutex;
    std::vector<CdevCallbackSetting> cdev_cb;

    ThermalUtils utils;
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // ANDROID_QTI_VENDOR_THERMAL_H
