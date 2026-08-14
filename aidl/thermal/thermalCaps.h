#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class ThermalCapsController {
public:
    ThermalCapsController();

    // Call with CPU temperature in millicelsius (e.g., 65000 = 65°C)
    void update(int64_t cpu_temp_mC);

private:
    void loadOnceLocked();
    void applyLevelLocked(int level);

    static bool readFile(const std::string& path, std::string* out);
    static bool writeFile(const std::string& path, const std::string& val);
    static bool readInt64(const std::string& path, int64_t* out);
    static std::vector<int64_t> parseIntList(const std::string& s);
    static int64_t clampToAvail(int64_t target, const std::vector<int64_t>& avail);

    std::mutex mLock;
    bool mLoaded = false;

    // CPU sysfs paths
    const std::string CPU0_MAX      = "/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq";
    const std::string CPU4_MAX      = "/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq";
    const std::string CPU0_INFO_MAX = "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq";
    const std::string CPU4_INFO_MAX = "/sys/devices/system/cpu/cpufreq/policy4/cpuinfo_max_freq";

    // GPU core devfreq (kgsl-3d0)
    const std::string GPU_DIR   = "/sys/class/devfreq/5000000.qcom,kgsl-3d0";
    const std::string GPU_MAX   = GPU_DIR + "/max_freq";
    const std::string GPU_AVAIL = GPU_DIR + "/available_frequencies";

    // GPU bandwidth devfreq (gpubw)
    const std::string GPUBW_DIR   = "/sys/class/devfreq/soc:qcom,gpubw";
    const std::string GPUBW_MAX   = GPUBW_DIR + "/max_freq";
    const std::string GPUBW_AVAIL = GPUBW_DIR + "/available_frequencies";

    // DDR bandwidth devfreq (cpu-cpu-ddr-bw)
    const std::string DDR_DIR   = "/sys/class/devfreq/soc:qcom,cpu-cpu-ddr-bw";
    const std::string DDR_MAX   = DDR_DIR + "/max_freq";
    const std::string DDR_AVAIL = DDR_DIR + "/available_frequencies";

    // Cached values
    int64_t mCpu0HwMax = 1804800;
    int64_t mCpu4HwMax = 2208000;

    std::vector<int64_t> mGpuAvail;
    std::vector<int64_t> mGpuBwAvail;
    std::vector<int64_t> mDdrAvail;

    int mLastLevel = -1;

    // Thresholds (millicelsius)
    static constexpr int64_t T0   = 65000;  // Start throttling
    static constexpr int64_t T1   = 72000;
    static constexpr int64_t T2   = 78000;
    static constexpr int64_t T3   = 83000;  // Emergency
    static constexpr int64_t HYST = 3000;   // 3°C hysteresis
};
