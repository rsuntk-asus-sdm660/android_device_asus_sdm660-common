/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Thermal HAL AIDL service entry point.
 * Migrated from HIDL service.cpp.
 */

#define LOG_TAG "thermal_hal_asus"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "thermal.h"

using aidl::android::hardware::thermal::implementation::Thermal;

int main() {
    LOG(INFO) << "Thermal HAL AIDL service starting...";

    ABinderProcess_setThreadPoolMaxThreadCount(0);

    auto thermal = ndk::SharedRefBase::make<Thermal>();
    if (thermal == nullptr) {
        LOG(FATAL) << "Error creating Thermal HAL instance. Exiting...";
        return EXIT_FAILURE;
    }

    const std::string instance =
        std::string(Thermal::descriptor) + "/default";

    auto status = AServiceManager_addService(
        thermal->asBinder().get(), instance.c_str());

    if (status != STATUS_OK) {
        LOG(FATAL) << "Could not register ThermalHAL (" << status << ")";
        return EXIT_FAILURE;
    }

    LOG(INFO) << "Thermal HAL AIDL service registered: " << instance;
    ABinderProcess_joinThreadPool();

    LOG(ERROR) << "Thermal HAL service exited unexpectedly";
    return EXIT_FAILURE;
}
