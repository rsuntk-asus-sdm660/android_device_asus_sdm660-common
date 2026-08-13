/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_THERMAL_MONITOR_H__
#define THERMAL_THERMAL_MONITOR_H__

#include <functional>
#include <string>
#include <thread>

namespace aidl::android::hardware::thermal::implementation {

using ueventMonitorCB = std::function<void(std::string sensor_name, int temp)>;

class ThermalMonitor {
public:
    ThermalMonitor(const ueventMonitorCB &inp_cb);
    ~ThermalMonitor();

    void parse_and_notify(char *inp_buf, ssize_t len);
    bool stopPolling() {
        return monitor_shutdown;
    }
    void start();

private:
    std::thread th;
    bool monitor_shutdown;
    ueventMonitorCB cb;
};

}  // namespace aidl::android::hardware::thermal::implementation

#endif  // THERMAL_THERMAL_MONITOR_H__
