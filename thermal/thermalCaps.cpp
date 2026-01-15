#include "thermalCaps.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <android-base/logging.h>

ThermalCapsController::ThermalCapsController() {}

bool ThermalCapsController::readFile(const std::string& path, std::string* out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    while (!out->empty() &&
           (out->back() == '\n' || out->back() == '\r' ||
            out->back() == ' '  || out->back() == '\t')) {
        out->pop_back();
    }
    return true;
}

bool ThermalCapsController::writeFile(const std::string& path, const std::string& val) {
    std::ofstream f(path);
    if (!f.is_open()) {
        LOG(WARNING) << "ThermalCaps: failed to open " << path << " for writing";
        return false;
    }
    f << val;
    return f.good();
}

bool ThermalCapsController::readInt64(const std::string& path, int64_t* out) {
    std::string s;
    if (!readFile(path, &s)) return false;
    if (s.empty()) return false;
    char* end = nullptr;
    errno = 0;
    long long v = std::strtoll(s.c_str(), &end, 10);
    if (errno != 0 || end == s.c_str()) return false;
    *out = static_cast<int64_t>(v);
    return true;
}

std::vector<int64_t> ThermalCapsController::parseIntList(const std::string& s) {
    std::vector<int64_t> vals;
    std::stringstream ss(s);
    int64_t v;
    while (ss >> v) {
        vals.push_back(v);
    }
    return vals;
}

int64_t ThermalCapsController::clampToAvail(int64_t target, const std::vector<int64_t>& avail) {
    if (avail.empty()) return target;
    int64_t best = 0;
    for (auto f : avail) {
        if (f <= target && f >= best) best = f;
    }
    if (best == 0) {
        // All freqs are above target, pick smallest
        best = *std::min_element(avail.begin(), avail.end());
    }
    return best;
}

void ThermalCapsController::loadOnceLocked() {
    if (mLoaded) return;
    mLoaded = true;

    // CPU hw max
    int64_t v = 0;
    if (readInt64(CPU0_INFO_MAX, &v)) mCpu0HwMax = v;
    if (readInt64(CPU4_INFO_MAX, &v)) mCpu4HwMax = v;

    // GPU available frequencies
    std::string s;
    if (readFile(GPU_AVAIL, &s)) {
        mGpuAvail = parseIntList(s);
        std::sort(mGpuAvail.begin(), mGpuAvail.end());
    }

    // GPU BW available frequencies
    if (readFile(GPUBW_AVAIL, &s)) {
        mGpuBwAvail = parseIntList(s);
        std::sort(mGpuBwAvail.begin(), mGpuBwAvail.end());
    }

    // DDR BW available frequencies
    if (readFile(DDR_AVAIL, &s)) {
        mDdrAvail = parseIntList(s);
        std::sort(mDdrAvail.begin(), mDdrAvail.end());
    }

    LOG(INFO) << "ThermalCaps: loaded"
              << " cpu0_hwmax=" << mCpu0HwMax
              << " cpu4_hwmax=" << mCpu4HwMax
              << " gpu_avail=" << mGpuAvail.size()
              << " gpubw_avail=" << mGpuBwAvail.size()
              << " ddr_avail=" << mDdrAvail.size();
}

void ThermalCapsController::applyLevelLocked(int level) {
    // ═══════════════════════════════════════════════════════════════
    // Helper: pick frequency at given fraction of max
    // ═══════════════════════════════════════════════════════════════
    auto pickFreq = [](const std::vector<int64_t>& avail, double frac) -> int64_t {
        if (avail.empty()) return 0;
        int64_t maxf = avail.back(); // sorted ascending
        int64_t target = static_cast<int64_t>(maxf * frac);
        return clampToAvail(target, avail);
    };

    auto getMax = [](const std::vector<int64_t>& avail) -> int64_t {
        if (avail.empty()) return 0;
        return avail.back();
    };

    // ═══════════════════════════════════════════════════════════════
    // CPU caps table (kHz)
    // ═══════════════════════════════════════════════════════════════
    int64_t cpu0max = mCpu0HwMax;
    int64_t cpu4max = mCpu4HwMax;

    // ═══════════════════════════════════════════════════════════════
    // GPU / GPU BW / DDR BW fractions per level
    // ═══════════════════════════════════════════════════════════════
    double gpuFrac   = 1.0;
    double gpuBwFrac = 1.0;
    double ddrFrac   = 1.0;

    switch (level) {
        case 0:
            // No throttling — full performance
            break;

        case 1:
            // Mild throttling
            cpu0max  = 1401600;
            cpu4max  = 1804800;
            gpuFrac  = 0.85;
            gpuBwFrac = 0.85;
            ddrFrac  = 0.80;
            break;

        case 2:
            // Medium throttling
            cpu0max  = 1113600;
            cpu4max  = 1401600;
            gpuFrac  = 0.70;
            gpuBwFrac = 0.70;
            ddrFrac  = 0.65;
            break;

        case 3:
            // Strong throttling
            cpu0max  = 998400;
            cpu4max  = 1113600;
            gpuFrac  = 0.55;
            gpuBwFrac = 0.55;
            ddrFrac  = 0.50;
            break;

        default: // level >= 4
            // Emergency throttling
            cpu0max  = 806400;
            cpu4max  = 902400;
            gpuFrac  = 0.40;
            gpuBwFrac = 0.40;
            ddrFrac  = 0.40;
            break;
    }

    // ═══════════════════════════════════════════════════════════════
    // Apply CPU caps
    // ═══════════════════════════════════════════════════════════════
    writeFile(CPU0_MAX, std::to_string(cpu0max));
    writeFile(CPU4_MAX, std::to_string(cpu4max));

    // ═══════════════════════════════════════════════════════════════
    // Apply GPU core cap
    // ═══════════════════════════════════════════════════════════════
    if (!mGpuAvail.empty()) {
        int64_t gpuMax = (level == 0) ? getMax(mGpuAvail)
                                      : pickFreq(mGpuAvail, gpuFrac);
        if (gpuMax > 0) writeFile(GPU_MAX, std::to_string(gpuMax));
    }

    // ═══════════════════════════════════════════════════════════════
    // Apply GPU bandwidth cap
    // ═══════════════════════════════════════════════════════════════
    if (!mGpuBwAvail.empty()) {
        int64_t gpuBwMax = (level == 0) ? getMax(mGpuBwAvail)
                                        : pickFreq(mGpuBwAvail, gpuBwFrac);
        if (gpuBwMax > 0) writeFile(GPUBW_MAX, std::to_string(gpuBwMax));
    }

    // ═══════════════════════════════════════════════════════════════
    // Apply DDR bandwidth cap
    // ═══════════════════════════════════════════════════════════════
    if (!mDdrAvail.empty()) {
        int64_t ddrMax = (level == 0) ? getMax(mDdrAvail)
                                      : pickFreq(mDdrAvail, ddrFrac);
        if (ddrMax > 0) writeFile(DDR_MAX, std::to_string(ddrMax));
    }

    LOG(INFO) << "ThermalCaps: applied level=" << level
              << " cpu0=" << cpu0max
              << " cpu4=" << cpu4max
              << " gpuFrac=" << gpuFrac
              << " ddrFrac=" << ddrFrac;
}

void ThermalCapsController::update(int64_t t) {
    std::lock_guard<std::mutex> lk(mLock);
    loadOnceLocked();

    // ═══════════════════════════════════════════════════════════════
    // Determine level based on temperature
    // ═══════════════════════════════════════════════════════════════
    int level = 0;
    if (t >= T3)      level = 4;
    else if (t >= T2) level = 3;
    else if (t >= T1) level = 2;
    else if (t >= T0) level = 1;

    // ═══════════════════════════════════════════════════════════════
    // Hysteresis for downshift (prevent oscillation)
    // ═══════════════════════════════════════════════════════════════
    if (mLastLevel >= 0 && level < mLastLevel) {
        int64_t cool = 0;
        switch (mLastLevel) {
            case 4: cool = T3 - HYST; break;
            case 3: cool = T2 - HYST; break;
            case 2: cool = T1 - HYST; break;
            case 1: cool = T0 - HYST; break;
            default: break;
        }
        if (t > cool) {
            level = mLastLevel; // Stay at current level
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // Apply caps if level changed
    // ═══════════════════════════════════════════════════════════════
    if (level != mLastLevel) {
        applyLevelLocked(level);
        mLastLevel = level;
    }
}
