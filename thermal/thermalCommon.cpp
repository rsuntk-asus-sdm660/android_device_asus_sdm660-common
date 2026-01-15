/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above
 *	  copyright notice, this list of conditions and the following
 *	  disclaimer in the documentation and/or other materials provided
 *	  with the distribution.
 *	* Neither the name of The Linux Foundation nor the names of its
 *	  contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdio>
#include <cinttypes>
#include <string>
#include <dirent.h>
#include <unordered_map>
#include <fstream>
#include <cstdlib>

#include <android-base/logging.h>
#include "thermalCommon.h"
#include "thermalCaps.h"

#define MAX_LENGTH		50
#define MAX_PATH		(256)
#define DEFAULT_HYSTERESIS	5000
#define THERMAL_SYSFS		"/sys/class/thermal/"
#define TZ_DIR_NAME		"thermal_zone"
#define TZ_DIR_FMT		"thermal_zone%d"
#define TEMPERATURE_FILE_FORMAT	"/sys/class/thermal/thermal_zone%d/temp"
#define POLICY_FILE_FORMAT	"/sys/class/thermal/thermal_zone%d/policy"
#define TRIP_FILE_FORMAT	"/sys/class/thermal/thermal_zone%d/trip_point_1_temp"
#define HYST_FILE_FORMAT	"/sys/class/thermal/thermal_zone%d/trip_point_1_hyst"
#define USER_SPACE_POLICY	"user_space"
#define TZ_TYPE			"type"
#define CDEV_DIR_NAME		"cooling_device"
#define CDEV_DIR_FMT		"cooling_device%d"
#define CDEV_CUR_STATE_PATH	"/sys/class/thermal/cooling_device%d/cur_state"
#define CPU_USAGE_FILE		"/proc/stat"
#define CPU_ONLINE_FILE_FORMAT	"/sys/devices/system/cpu/cpu%d/online"

namespace android {
namespace hardware {
namespace thermal {
namespace V2_0 {
namespace implementation {

// ═══════════════════════════════════════════════════════════════
// Global thermal caps controller instance
// ═══════════════════════════════════════════════════════════════
static ThermalCapsController gCaps;

/*
 * Cooling device type mapping
 * Updated for kernel 4.19 with actual device cooling_device types
 */
static std::unordered_map<std::string, cdevType> cdev_map = {
	// CPU frequency cooling
	{"thermal-cpufreq-0", cdevType::CPU},
	{"thermal-cpufreq-1", cdevType::CPU},
	{"thermal-cpufreq-2", cdevType::CPU},
	{"thermal-cpufreq-3", cdevType::CPU},
	{"thermal-cpufreq-4", cdevType::CPU},
	{"thermal-cpufreq-5", cdevType::CPU},
	{"thermal-cpufreq-6", cdevType::CPU},
	{"thermal-cpufreq-7", cdevType::CPU},
	// CPU cluster cooling
	{"thermal-cluster-4-7", cdevType::CPU},
	{"thermal-cluster-0-3", cdevType::CPU},
	// CPU isolation
	{"cpu-isolate0", cdevType::CPU},
	{"cpu-isolate1", cdevType::CPU},
	{"cpu-isolate2", cdevType::CPU},
	{"cpu-isolate3", cdevType::CPU},
	{"cpu-isolate4", cdevType::CPU},
	{"cpu-isolate5", cdevType::CPU},
	{"cpu-isolate6", cdevType::CPU},
	{"cpu-isolate7", cdevType::CPU},
	// GPU cooling
	{"thermal-devfreq-0", cdevType::GPU},
	{"gpu", cdevType::GPU},
	// Modem cooling
	{"modem_tj", cdevType::MODEM},
	{"modem_skin", cdevType::MODEM},
	// DSP cooling
	{"cdsp", cdevType::NPU},
	{"cdsp_hw", cdevType::NPU},
	// Battery cooling
	{"battery", cdevType::BATTERY},
	{"charge_cdev", cdevType::BATTERY},
};

ThermalCommon::ThermalCommon()
{
	LOG(DEBUG) << "Entering " << __func__;
	ncpus = (int)sysconf(_SC_NPROCESSORS_CONF);
	if (ncpus < 1)
		LOG(ERROR) << "Error retrieving number of cores";
}

static int writeToFile(std::string_view path, std::string data)
{
	std::fstream outFile;

	outFile.open(std::string(path).c_str(),
			std::fstream::binary | std::fstream::out);
	if (outFile.is_open()) {
		LOG(DEBUG) << "writing: "<< data << " in path:" << path
			<< std::endl;
		outFile << data;
		outFile.close();
		return data.length();
	}

	LOG(ERROR) << "Error opening file: "<< path << std::endl;
	return -1;
}

static int readLineFromFile(std::string_view path, std::string& out)
{
	char *fgets_ret;
	FILE *fd;
	int rv;
	char buf[MAX_LENGTH];

	out.clear();

	fd = fopen(std::string(path).c_str(), "r");
	if (fd == NULL) {
		LOG(ERROR) << "Path:" << std::string(path) << " file open error.err:"
			<< strerror(errno) << std::endl;
		return -errno;
	}

	fgets_ret = fgets(buf, MAX_LENGTH, fd);
	if (NULL != fgets_ret) {
		rv = (int)strlen(buf);
		out.append(buf, rv);
	} else {
		rv = ferror(fd);
	}

	fclose(fd);
	out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
	LOG(DEBUG) << "Path:" << std::string(path) << " Val:" << out << std::endl;

	return rv;
}

int ThermalCommon::readFromFile(std::string_view path, std::string& out)
{
	return readLineFromFile(path, out);
}

static int get_tzn(std::string sensor_name)
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	int found = -1;
	int tzn = 0;
	char name[MAX_PATH] = {0};
	char cwd[MAX_PATH] = {0};
	int ret = 0;

	if (!getcwd(cwd, sizeof(cwd)))
		return found;

	ret = chdir(THERMAL_SYSFS);
	if (ret) {
		LOG(ERROR) << "Unable to change to " << THERMAL_SYSFS << std::endl;
		return found;
	}
	tdir = opendir(THERMAL_SYSFS);
	if (!tdir) {
		LOG(ERROR) << "Unable to open " << THERMAL_SYSFS << std::endl;
		return found;
	}

	while ((tdirent = readdir(tdir))) {
		std::string buf;

		if (strncmp(tdirent->d_name, TZ_DIR_NAME,
			strlen(TZ_DIR_NAME)) != 0)
			continue;

		snprintf(name, MAX_PATH, "%s%s/%s", THERMAL_SYSFS,
				tdirent->d_name, TZ_TYPE);
		ret = readLineFromFile(std::string_view(name), buf);
		if (ret <= 0) {
			LOG(ERROR) <<
				"get_tzn: sensor name read error for tz:" <<
				tdirent->d_name << std::endl;
			continue;
		}
		if (!strncmp(buf.c_str(), sensor_name.c_str(),
					sensor_name.length())) {
			found = 1;
			break;
		}
	}

	if (found == 1) {
		sscanf(tdirent->d_name, TZ_DIR_FMT, &tzn);
		LOG(DEBUG) << "Sensor: " << sensor_name <<
			" found at tz: " << tzn << std::endl;
		found = tzn;
	}

	closedir(tdir);
	ret = chdir(cwd);

	return found;
}

int ThermalCommon::initialize_sensor(struct target_therm_cfg& cfg, int sens_idx)
{
	struct therm_sensor sensor;
	int idx = 0;

	sensor.tzn = get_tzn(cfg.sensor_list[sens_idx]);
	if (sensor.tzn < 0) {
		LOG(ERROR) << "No thermal zone for sensor: " <<
			cfg.sensor_list[sens_idx] << ", ret:" <<
			sensor.tzn << std::endl;
		return -1;
	}
	if (cfg.type == TemperatureType::CPU)
		sensor.thresh.name = sensor.t.name =
			std::string("CPU") + std::to_string(sens_idx);
	else
		sensor.thresh.name = sensor.t.name = cfg.label;

	/*
	 * Multiplier handling for different sensor types:
	 * ══════════════════════════════════════════════════════════════════
	 * BCL_PERCENTAGE (soc):     raw value (100 = 100%), mulFactor = 1
	 * BCL_CURRENT (ibat):       raw mA value, mulFactor = 1
	 * BCL_VOLTAGE (vbat):       raw mV value, mulFactor = 1
	 * BATTERY (battery/bms):    milliCelsius (31000 = 31.0°C), mulFactor = 1000
	 * CPU/GPU/SKIN/others:      milliCelsius, mulFactor = 1000
	 * ══════════════════════════════════════════════════════════════════
	 */
	if (cfg.type == TemperatureType::BCL_PERCENTAGE ||
	    cfg.type == TemperatureType::BCL_CURRENT ||
	    cfg.type == TemperatureType::BCL_VOLTAGE) {
		sensor.mulFactor = 1;
	} else {
		// All temperature sensors report in milliCelsius
		sensor.mulFactor = 1000;
	}

	sensor.sensor_name = cfg.sensor_list[sens_idx];
	sensor.positiveThresh = cfg.positive_thresh_ramp;
	sensor.lastThrottleStatus = sensor.t.throttlingStatus =
		ThrottlingSeverity::NONE;
	sensor.thresh.type = sensor.t.type = cfg.type;
	sensor.thresh.vrThrottlingThreshold = UNKNOWN_TEMPERATURE;
	
	for (idx = 0; idx <= (size_t)ThrottlingSeverity::SHUTDOWN; idx++) {
		sensor.thresh.hotThrottlingThresholds[idx] =
		sensor.thresh.coldThrottlingThresholds[idx] =
			UNKNOWN_TEMPERATURE;
	}

	if (cfg.throt_thresh != 0 && cfg.positive_thresh_ramp)
		sensor.thresh.hotThrottlingThresholds[(size_t)ThrottlingSeverity::SEVERE] =
			cfg.throt_thresh / (float)sensor.mulFactor;
	else if (cfg.throt_thresh != 0 && !cfg.positive_thresh_ramp)
		sensor.thresh.coldThrottlingThresholds[(size_t)ThrottlingSeverity::SEVERE] =
			cfg.throt_thresh / (float)sensor.mulFactor;

	if (cfg.shutdwn_thresh != 0 && cfg.positive_thresh_ramp)
		sensor.thresh.hotThrottlingThresholds[(size_t)ThrottlingSeverity::SHUTDOWN] =
			cfg.shutdwn_thresh / (float)sensor.mulFactor;
	else if (cfg.shutdwn_thresh != 0 && !cfg.positive_thresh_ramp)
		sensor.thresh.coldThrottlingThresholds[(size_t)ThrottlingSeverity::SHUTDOWN] =
			cfg.shutdwn_thresh / (float)sensor.mulFactor;

	if (cfg.vr_thresh != 0)
		sensor.thresh.vrThrottlingThreshold =
			cfg.vr_thresh / (float)sensor.mulFactor;
	
	sens.push_back(sensor);

	LOG(INFO) << "ThermalHAL: Initialized sensor '" << sensor.t.name 
	          << "' (" << sensor.sensor_name << ") at tz" << sensor.tzn
	          << " mulFactor=" << sensor.mulFactor;

	return 0;
}

int ThermalCommon::initializeCpuSensor(struct target_therm_cfg& cpu_cfg)
{
	int cpu = 0;

	for (; cpu < ncpus; cpu++) {
		if (cpu >= (int)cpu_cfg.sensor_list.size()) {
			LOG(WARNING) << "ThermalHAL: Not enough CPU sensors defined for CPU" << cpu;
			break;
		}
		if (initialize_sensor(cpu_cfg, cpu) < 0)
			return -1;
	}

	return 0;
}

int ThermalCommon::initThermalZones(std::vector<struct target_therm_cfg>& cfg)
{
	std::vector<struct target_therm_cfg>::iterator it;

	if (cfg.empty()) {
		LOG(ERROR) << std::string(__func__) + ":Invalid input";
		return -1;
	}

	for (it = cfg.begin(); it != cfg.end(); it++)
	{
		if (it->type == TemperatureType::CPU) {
			if (initializeCpuSensor(*it) < 0)
				return -1;
			continue;
		}
		if (initialize_sensor(*it, 0) < 0) {
			LOG(WARNING) << "ThermalHAL: Failed to init sensor: " 
			             << it->label << ", skipping";
			continue;  // Don't fail completely, skip unavailable sensors
		}
	}

	LOG(INFO) << "ThermalHAL: Initialized " << sens.size() << " thermal sensors";
	return sens.size();
}

int ThermalCommon::initCdev()
{
	DIR *tdir = NULL;
	struct dirent *tdirent = NULL;
	int cdevn = 0;
	char name[MAX_PATH] = {0};
	char cwd[MAX_PATH] = {0};
	int ret = 0;

	if (!getcwd(cwd, sizeof(cwd)))
		return 0;

	ret = chdir(THERMAL_SYSFS);
	if (ret) {
		LOG(ERROR) << "Unable to change to " << THERMAL_SYSFS << std::endl;
		return 0;
	}
	tdir = opendir(THERMAL_SYSFS);
	if (!tdir) {
		LOG(ERROR) << "Unable to open " << THERMAL_SYSFS << std::endl;
		return 0;
	}

	while ((tdirent = readdir(tdir))) {
		std::string buf;
		std::unordered_map<std::string, cdevType>::iterator it;
		struct therm_cdev cdevInst;

		if (strncmp(tdirent->d_name, CDEV_DIR_NAME,
			strlen(CDEV_DIR_NAME)) != 0)
			continue;

		snprintf(name, MAX_PATH, "%s%s/%s", THERMAL_SYSFS,
				tdirent->d_name, TZ_TYPE);
		ret = readLineFromFile(std::string_view(name), buf);
		if (ret <= 0) {
			LOG(DEBUG) <<
				"init_cdev: cdev type read error for cdev:" <<
				tdirent->d_name << std::endl;
			continue;
		}
		it = cdev_map.find(buf);
		if (it == cdev_map.end()) {
			LOG(DEBUG) << "ThermalHAL: Unknown cooling device type: " << buf;
			continue;
		}
		sscanf(tdirent->d_name, CDEV_DIR_FMT, &cdevn);
		LOG(DEBUG) << "cdev: " << it->first <<
			" found at cdev number: " << cdevn << std::endl;
		cdevInst.c.name = it->first;
		cdevInst.c.type = it->second;
		cdevInst.cdevn = cdevn;
		read_cdev_state(cdevInst);
		cdev.push_back(cdevInst);
	}

	closedir(tdir);
	ret = chdir(cwd);

	LOG(INFO) << "ThermalHAL: Initialized " << cdev.size() << " cooling devices";
	return cdev.size();
}

int ThermalCommon::read_cdev_state(struct therm_cdev& cdev)
{
	char file_name[MAX_PATH];
	std::string buf;
	int ret = 0;

	LOG(DEBUG) << "Entering " << __func__;
	snprintf(file_name, sizeof(file_name), CDEV_CUR_STATE_PATH,
			cdev.cdevn);
	ret = readLineFromFile(std::string(file_name), buf);
	if (ret <= 0) {
		LOG(ERROR) << "Cdev state read error:" << ret <<
			" for cdev: " << cdev.c.name;
		return -1;
	}
	cdev.c.value = std::atoi(buf.c_str());
	LOG(DEBUG) << "cdev Name:" << cdev.c.name << ". state:" <<
		cdev.c.value << std::endl;

	return cdev.c.value;
}

int ThermalCommon::estimateSeverity(struct therm_sensor& sensor)
{
	int idx = 0;
	ThrottlingSeverity severity = ThrottlingSeverity::NONE;
	float temp = sensor.t.value;

	for (idx = (int)ThrottlingSeverity::SHUTDOWN; idx >= 0; idx--) {
		if (idx == (int)sensor.t.throttlingStatus) {
			if ((sensor.positiveThresh &&
				!isnan(sensor.thresh.hotThrottlingThresholds[idx]) &&
				temp >=
				(sensor.thresh.hotThrottlingThresholds[idx] -
				DEFAULT_HYSTERESIS / sensor.mulFactor)) ||
				(!sensor.positiveThresh &&
				!isnan(sensor.thresh.coldThrottlingThresholds[idx]) &&
				temp <=
				(sensor.thresh.coldThrottlingThresholds[idx] +
				DEFAULT_HYSTERESIS / sensor.mulFactor)))
				break;
			continue;
		}
		if ((sensor.positiveThresh &&
			!isnan(sensor.thresh.hotThrottlingThresholds[idx]) &&
			temp >=
			sensor.thresh.hotThrottlingThresholds[idx]) ||
		 	(!sensor.positiveThresh &&
			!isnan(sensor.thresh.coldThrottlingThresholds[idx]) &&
			temp <=
			sensor.thresh.coldThrottlingThresholds[idx]))
			break;
	}
	if (idx >= 0)
		severity = (ThrottlingSeverity)(idx);
	LOG(DEBUG) << "Sensor Name:" << sensor.t.name << ". prev severity:" <<
		(int)sensor.lastThrottleStatus << ". cur severity:" <<
		(int)sensor.t.throttlingStatus << " New severity:" <<
		(int)severity << std::endl;
	if (severity == sensor.t.throttlingStatus)
		return -1;
	sensor.lastThrottleStatus = sensor.t.throttlingStatus;
	sensor.t.throttlingStatus = severity;

	return (int)severity;
}

int ThermalCommon::read_temperature(struct therm_sensor& sensor)
{
	char file_name[MAX_PATH];
	std::string buf;
	int ret = 0;

	LOG(DEBUG) << "Entering " << __func__;
	snprintf(file_name, sizeof(file_name), TEMPERATURE_FILE_FORMAT,
			sensor.tzn);
	ret = readLineFromFile(std::string(file_name), buf);
	if (ret <= 0) {
		LOG(ERROR) << "Temperature read error:" << ret <<
			" for sensor " << sensor.t.name;
		return -1;
	}
	sensor.t.value = (float)std::atoi(buf.c_str()) / (float)sensor.mulFactor;

	// ═══════════════════════════════════════════════════════════════
	// Apply thermal caps (CPU + GPU + DDR) based on CPU temperature
	// Trigger only once per poll cycle using CPU0 on cpuss-0-usr
	// ═══════════════════════════════════════════════════════════════
	if (sensor.thresh.type == TemperatureType::CPU &&
	    sensor.sensor_name == "cpuss-0-usr" &&
	    sensor.t.name == "CPU0") {

		const int64_t temp_mC = std::atoll(buf.c_str());
		gCaps.update(temp_mC);
	}

	LOG(DEBUG) << "Sensor Name:" << sensor.t.name << ". Temperature:" <<
		(float)sensor.t.value << std::endl;

	return ret;
}

void ThermalCommon::initThreshold(struct therm_sensor& sensor)
{
	char file_name[MAX_PATH] = "";
	std::string buf;
	int ret = 0, idx;
	int next_trip, curr_trip, hyst_temp = 0;

	LOG(DEBUG) << "Entering " << __func__;
	if (!sensor.positiveThresh) {
		LOG(DEBUG) << "Negative temperature ramp for sensor:" <<
			sensor.t.name << ", skipping threshold init";
		return;
	}
	snprintf(file_name, sizeof(file_name), POLICY_FILE_FORMAT,
			sensor.tzn);
	ret = readLineFromFile(std::string(file_name), buf);
	if (ret <= 0) {
		LOG(DEBUG) << "Policy read error:" << ret <<
			" for sensor " << sensor.t.name;
		return;
	}
	if (buf != std::string(USER_SPACE_POLICY)) {
		LOG(DEBUG) << "Policy:" << buf << " for sensor:" <<
			sensor.t.name << " (not user_space, skipping)";
		return;
	}

	next_trip = UNKNOWN_TEMPERATURE;
	for (idx = 0; idx <= (int)ThrottlingSeverity::SHUTDOWN; idx++) {
		if (isnan(sensor.thresh.hotThrottlingThresholds[idx])
			|| idx <= (int)sensor.t.throttlingStatus)
			continue;

		next_trip = sensor.thresh.hotThrottlingThresholds[idx] *
				sensor.mulFactor;
		break;
	}

	if (!isnan(next_trip)) {
		LOG(DEBUG) << "Sensor: " << sensor.t.name << " high trip:"
			<< next_trip << std::endl;
		snprintf(file_name, sizeof(file_name), TRIP_FILE_FORMAT,
				sensor.tzn);
		writeToFile(std::string_view(file_name), std::to_string(next_trip));
	}
	if (sensor.t.throttlingStatus != ThrottlingSeverity::NONE) {
		curr_trip = sensor.thresh.hotThrottlingThresholds[
				(int)sensor.t.throttlingStatus]
					* sensor.mulFactor;
		if (!isnan(next_trip))
			hyst_temp = (next_trip - curr_trip) + DEFAULT_HYSTERESIS;
		else
			hyst_temp = DEFAULT_HYSTERESIS;
		LOG(DEBUG) << "Sensor: " << sensor.t.name << " hysteresis:"
			<< hyst_temp << std::endl;
		snprintf(file_name, sizeof(file_name), HYST_FILE_FORMAT,
				sensor.tzn);
		writeToFile(std::string_view(file_name), std::to_string(hyst_temp));
	}

	return;
}

int ThermalCommon::get_cpu_usages(hidl_vec<CpuUsage>& list) {
	int vals, cpu_num, online;
	ssize_t read;
	uint64_t user, nice, system, idle, active, total;
	char *line = NULL;
	size_t len = 0;
	size_t cpu = 0;
	char file_name[MAX_LENGTH];
	FILE *file;
	FILE *cpu_file;

	list.resize(ncpus);
	file = fopen(CPU_USAGE_FILE, "r");
	if (file == NULL) {
		LOG(ERROR) << "failed to open:" << CPU_USAGE_FILE <<
			" err:" << strerror(errno);
		return -errno;
	}

	while ((read = getline(&line, &len, file)) != -1) {
		if (strnlen(line, read) < 4 || strncmp(line, "cpu", 3) != 0 ||
				!isdigit(line[3])) {
			free(line);
			line = NULL;
			len = 0;
			continue;
		}
		vals = sscanf(line, \
			"cpu%d %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, \
			&cpu_num, &user, &nice, &system, &idle);

		free(line);
		line = NULL;
		len = 0;

		if (vals != 5 || cpu == (size_t)ncpus) {
			if (vals != 5) {
				LOG(ERROR) <<
				"failed to read CPU information from file: "
				<< strerror(errno);
			} else {
				LOG(ERROR) <<
					"/proc/stat file has incorrect format.";
			}
			fclose(file);
			return errno ? -errno : -EIO;
		}

		active = user + nice + system;
		total = active + idle;

		snprintf(file_name, MAX_LENGTH, CPU_ONLINE_FILE_FORMAT,
				cpu_num);
		cpu_file = fopen(file_name, "r");
		online = 0;
		if (cpu_file == NULL) {
			// CPU0 is always online and may not have online file
			if (cpu_num == 0) {
				online = 1;
			} else {
				LOG(ERROR) << "failed to open file:" << file_name <<
					" err: " << strerror(errno);
				fclose(file);
				return -errno;
			}
		} else {
			if (1 != fscanf(cpu_file, "%d", &online)) {
				LOG(ERROR) << "failed to read CPU online information" << strerror(errno);
				fclose(file);
				fclose(cpu_file);
				return errno ? -errno : -EIO;
			}
			fclose(cpu_file);
		}

		list[cpu_num].name = std::string("CPU") + std::to_string(cpu_num);
		list[cpu_num].active = active;
		list[cpu_num].total = total;
		list[cpu_num].isOnline = online;
		cpu++;
	}
	fclose(file);
	if (cpu != (size_t)ncpus) {
		LOG(ERROR) << "/proc/stat file has incorrect format.";
		return -EIO;
	}
	return ncpus;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace thermal
}  // namespace hardware
}  // namespace android
