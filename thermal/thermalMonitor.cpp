/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <cstring>

#include <android-base/logging.h>

#include "thermalMonitor.h"

#define UEVENT_BUF 1024

#define HYST_FMT "change@/devices/virtual/thermal/thermal_zone%d\n\
	ACTION=change\n\
	DEVPATH=/devices/virtual/thermal/thermal_zone%d\n\
	SUBSYSTEM=thermal\n\
	NAME=%s\n\
	TEMP=%d\n\
	HYST=%d\n\
	EVENT=%d\n"

#define TRIP_FMT "change@/devices/virtual/thermal/thermal_zone%d\n\
	ACTION=change\n\
	DEVPATH=/devices/virtual/thermal/thermal_zone%d\n\
	SUBSYSTEM=thermal\n\
	NAME=%s\n\
	TEMP=%d\n\
	TRIP=%d\n\
	EVENT=%d\n"

namespace aidl::android::hardware::thermal::implementation {

using parseCB = std::function<void(char *inp_buf, ssize_t len)>;
using pollCB = std::function<bool()>;

void thermal_monitor_uevent(const parseCB &parse_cb, const pollCB &stopPollCB) {
    struct pollfd pfd;
    char buf[UEVENT_BUF] = {0};
    int sz = 64 * 1024;
    struct sockaddr_nl nls;

    memset(&nls, 0, sizeof(nls));
    nls.nl_family = AF_NETLINK;
    nls.nl_pid = getpid();
    nls.nl_groups = 0xffffffff;

    pfd.events = POLLIN;
    pfd.fd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC,
                    NETLINK_KOBJECT_UEVENT);
    if (pfd.fd < 0) {
        LOG(ERROR) << "socket creation error:" << errno;
        return;
    }

    setsockopt(pfd.fd, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz));
    if (bind(pfd.fd, (struct sockaddr *)&nls, sizeof(nls)) < 0) {
        close(pfd.fd);
        LOG(ERROR) << "socket bind failed:" << errno;
        return;
    }
    LOG(DEBUG) << "Listening for uevent";

    while (!stopPollCB()) {
        ssize_t len;
        int err;

        err = poll(&pfd, 1, -1);
        if (err == -1) {
            LOG(ERROR) << "Error in uevent poll.";
            break;
        }
        if (stopPollCB()) {
            LOG(INFO) << "Exiting uevent monitor";
            return;
        }
        len = recv(pfd.fd, buf, sizeof(buf) - 1, MSG_DONTWAIT);
        if (len == -1) {
            LOG(ERROR) << "uevent read failed:" << errno;
            continue;
        }
        buf[len] = '\0';
        parse_cb(buf, len);
    }
}

ThermalMonitor::ThermalMonitor(const ueventMonitorCB &inp_cb)
    : cb(inp_cb) {
    monitor_shutdown = false;
}

ThermalMonitor::~ThermalMonitor() {
    monitor_shutdown = true;
    if (th.joinable())
        th.join();
}

void ThermalMonitor::start() {
    th = std::thread(thermal_monitor_uevent,
        std::bind(&ThermalMonitor::parse_and_notify, this,
                  std::placeholders::_1, std::placeholders::_2),
        std::bind(&ThermalMonitor::stopPolling, this));
}

void ThermalMonitor::parse_and_notify(char *inp_buf, ssize_t len) {
    int zone_num, temp, trip, ret = 0, event;
    ssize_t i = 0;
    char sensor_name[30] = "", buf[UEVENT_BUF] = {0};

    LOG(DEBUG) << "monitor received thermal uevent: " << inp_buf;

    while (i < len) {
        if (i >= UEVENT_BUF) return;
        ret = snprintf(buf + i, UEVENT_BUF - i, "%s ", inp_buf + i);
        if (ret == (int)(strlen(inp_buf + i) + 1))
            i += ret;
        else
            return;
    }

    if (!strstr(buf, "SUBSYSTEM=thermal"))
        return;

    if (strstr(buf, "TRIP=")) {
        ret = sscanf(buf, TRIP_FMT, &zone_num, &zone_num, sensor_name,
                     &temp, &trip, &event);
    } else {
        ret = sscanf(buf, HYST_FMT, &zone_num, &zone_num, sensor_name,
                     &temp, &trip, &event);
    }
    if (ret <= 0 || ret == EOF) {
        LOG(ERROR) << "read error:" << ret << ". buf:" << buf;
        return;
    }
    cb(sensor_name, temp);
}

}  // namespace aidl::android::hardware::thermal::implementation
