/*
 * ASUS X00TD/X01BD — UNIVERSAL ANDROID 13/14 — 2025
 * RAM variants: 3GB / 4GB / 6GB ONLY
 */

#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using std::string;

/* Dalvik VM */
static string heapstartsize, heapgrowthlimit, heapsize;
static string heaptargetutilization, heapminfree, heapmaxfree;

/* ZRAM */
static string zram_size, zram_streams, zram_swappiness, zram_algo;

/* LMKD */
static string lmk_psi_partial, lmk_psi_complete, lmk_swap_util;
static string lmk_thrashing_limit, lmk_thrashing_decay;
static string lmk_kill_heaviest, lmk_kill_timeout;
static string lmk_upgrade_pressure, lmk_downgrade_pressure;

/* RAM variant identifier */
static string ram_variant;

static void property_override(const char* prop, const char* value, bool add = true)
{
    prop_info* pi = (prop_info*)__system_property_find(prop);
    if (pi) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

static void configure_by_ram()
{
    struct sysinfo sys {};
    sysinfo(&sys);
    const long ram_mb = (sys.totalram * sys.mem_unit) / (1024L * 1024L);

    LOG(INFO) << "SDM660-X00TD: Detected available RAM: " << ram_mb << " MB";

    if (ram_mb >= 5000) {
        /* ═══════════════════════════════════════════════════════════
         *  6 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "6GB";
        LOG(INFO) << "Applying 6GB RAM profile";

        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "48m";

        zram_size = "2684354560";       // 2.5 GB
        zram_streams = "8";
        zram_swappiness = "100";
        zram_algo = "lz4";              // ← lz4 для всех

        lmk_psi_partial = "300";
        lmk_psi_complete = "700";
        lmk_swap_util = "90";
        lmk_thrashing_limit = "60";
        lmk_thrashing_decay = "20";
        lmk_kill_heaviest = "false";
        lmk_kill_timeout = "100";
        lmk_upgrade_pressure = "95";
        lmk_downgrade_pressure = "60";

    } else if (ram_mb >= 3400) {
        /* ═══════════════════════════════════════════════════════════
         *  4 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "4GB";
        LOG(INFO) << "Applying 4GB RAM profile";

        heapstartsize = "12m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "4m";
        heapmaxfree = "24m";

        zram_size = "2147483648";       // 2 GB
        zram_streams = "8";
        zram_swappiness = "90";
        zram_algo = "lz4";              // ← lz4 для всех

        lmk_psi_partial = "250";
        lmk_psi_complete = "650";
        lmk_swap_util = "85";
        lmk_thrashing_limit = "50";
        lmk_thrashing_decay = "15";
        lmk_kill_heaviest = "true";
        lmk_kill_timeout = "80";
        lmk_upgrade_pressure = "90";
        lmk_downgrade_pressure = "50";

    } else {
        /* ═══════════════════════════════════════════════════════════
         *  3 GB PROFILE
         * ═══════════════════════════════════════════════════════════ */
        ram_variant = "3GB";
        LOG(INFO) << "Applying 3GB RAM profile";

        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "384m";
        heaptargetutilization = "0.75";
        heapminfree = "2m";
        heapmaxfree = "8m";

        zram_size = "1610612736";       // 1.5 GB
        zram_streams = "8";
        zram_swappiness = "80";
        zram_algo = "lz4";              // ← lz4 для всех

        lmk_psi_partial = "200";
        lmk_psi_complete = "600";
        lmk_swap_util = "80";
        lmk_thrashing_limit = "40";
        lmk_thrashing_decay = "10";
        lmk_kill_heaviest = "true";
        lmk_kill_timeout = "60";
        lmk_upgrade_pressure = "85";
        lmk_downgrade_pressure = "40";
    }
    
    property_override("ro.boot.hardware.ram", ram_variant.c_str());
}

static void NFC_check()
{
    const char* nfc_path = "/proc/NFC_CHECK";
    std::ifstream infile;
    std::string line;
    int retries = 30;

    while (retries-- > 0) {
        infile.open(nfc_path);
        if (infile.is_open())
            break;
        usleep(100000);
    }

    if (infile.is_open() && getline(infile, line)) {
        infile.close();
        
        if (line.find("SUPPORTED") != std::string::npos) {
            property_override("ro.hq.support.nfc", "1");
            property_override("ro.boot.product.hardware.sku", "NFC");
            property_override("persist.sys.nfc.supported", "true");
            LOG(INFO) << "NFC: Hardware detected - ENABLED";
        } else {
            property_override("ro.hq.support.nfc", "0");
            property_override("ro.boot.product.hardware.sku", "");
            property_override("persist.sys.nfc.supported", "false");
            LOG(INFO) << "NFC: Not present - DISABLED";
        }
    } else {
        property_override("ro.hq.support.nfc", "0");
        property_override("ro.boot.product.hardware.sku", "");
        property_override("persist.sys.nfc.supported", "false");
        LOG(INFO) << "NFC: Detection failed - DISABLED";
    }
}

static void set_dalvik_properties()
{
    property_override("dalvik.vm.heapstartsize", heapstartsize.c_str());
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit.c_str());
    property_override("dalvik.vm.heapsize", heapsize.c_str());
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization.c_str());
    property_override("dalvik.vm.heapminfree", heapminfree.c_str());
    property_override("dalvik.vm.heapmaxfree", heapmaxfree.c_str());
    
    /* Дополнительные оптимизации Dalvik */
    property_override("dalvik.vm.dex2oat-threads", "8");
    property_override("dalvik.vm.image-dex2oat-threads", "8");
    property_override("dalvik.vm.usejit", "true");
    property_override("dalvik.vm.usejitprofiles", "true");
}

static void set_zram_properties()
{
    property_override("vendor.zram.size", zram_size.c_str());
    property_override("vendor.zram.streams", zram_streams.c_str());
    property_override("vendor.zram.swappiness", zram_swappiness.c_str());
    property_override("vendor.zram.comp_algorithm", zram_algo.c_str());
    
    /* Флаг для init.rc что ZRAM нужно настроить */
    property_override("vendor.zram.enabled", "true");
}

static void set_lmkd_properties()
{
    /*
     * ВАЖНО: Проверить kernel cmdline!
     * Если есть cgroup_disable=pressure - PSI не работает
     */
    property_override("ro.lmk.use_psi", "true");
    property_override("ro.lmk.use_minfree_levels", "false");
    
    property_override("ro.lmk.psi_partial_stall_ms", lmk_psi_partial.c_str());
    property_override("ro.lmk.psi_complete_stall_ms", lmk_psi_complete.c_str());
    property_override("ro.lmk.swap_util_max", lmk_swap_util.c_str());
    property_override("ro.lmk.thrashing_limit", lmk_thrashing_limit.c_str());
    property_override("ro.lmk.thrashing_limit_decay", lmk_thrashing_decay.c_str());
    property_override("ro.lmk.kill_heaviest_task", lmk_kill_heaviest.c_str());
    property_override("ro.lmk.kill_timeout_ms", lmk_kill_timeout.c_str());
    property_override("ro.lmk.upgrade_pressure", lmk_upgrade_pressure.c_str());
    property_override("ro.lmk.downgrade_pressure", lmk_downgrade_pressure.c_str());

    /* Фиксированные параметры */
    property_override("ro.lmk.swap_free_low_percentage", "15");
    property_override("ro.lmk.psi_scrit_complete_stall_ms", "400");
    property_override("ro.lmk.critical_upgrade", "true");
    property_override("ro.lmk.pressure_after_kill", "25");
    property_override("ro.lmk.filecache_min_kb", "153600");
    property_override("ro.lmk.stall_limit_critical", "50");

    /* Не low_ram устройства */
    property_override("ro.lmk.low_ram", "false");
    property_override("ro.config.low_ram", "false");

    /* Debug off для production */
    property_override("ro.lmk.debug", "false");
    property_override("ro.lmk.log_stats", "false");
}

static void set_performance_properties()
{
    /* Surfaceflinger */
    property_override("debug.sf.latch_unsignaled", "0");
    property_override("debug.sf.disable_backpressure", "1");
    
    /* Renderer */
    property_override("debug.hwui.renderer", "skiagl");
    property_override("renderthread.skia.reduceopstasksplitting", "true");
    
    /* Disable unnecessary logging */
    property_override("persist.traced.enable", "0");
}

void vendor_load_properties()
{
    configure_by_ram();
    NFC_check();

    set_dalvik_properties();
    set_zram_properties();
    set_lmkd_properties();
    set_performance_properties();

    LOG(INFO) << "SDM660-X00TD: vendor properties loaded for " << ram_variant << " variant";
}
