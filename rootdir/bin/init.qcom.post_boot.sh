#!/vendor/bin/sh

# ═══════════════════════════════════════════════════════════════════════════
# init.qcom.post_boot.sh for SDM660/SDM636 on Kernel 4.19 (EAS)
# Optimized for stability under load + thermal caps integration
# ═══════════════════════════════════════════════════════════════════════════

function sdm660_sched_schedutil_dcvs() {
    # ═══════════════════════════════════════════════════════════════
    # 1. CPU FREQUENCY GOVERNOR (SCHEDUTIL)
    # ═══════════════════════════════════════════════════════════════

    # Little Cluster (CPU 0-3)
    echo "schedutil" > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
    echo 500 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/up_rate_limit_us
    echo 20000 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/down_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/iowait_boost_enable

    # ★ ИЗМЕНЕНО: Низкий baseline min_freq для работы thermal caps
    echo 576000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq

    # Big Cluster (CPU 4-7)
    echo "schedutil" > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor
    echo 1000 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/up_rate_limit_us
    echo 20000 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/down_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/iowait_boost_enable

    # ★ ИЗМЕНЕНО: Низкий baseline min_freq для работы thermal caps
    echo 825600 > /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq

    # ═══════════════════════════════════════════════════════════════
    # 2. SCHEDTUNE (EAS BOOST)
    # ★ ИЗМЕНЕНО: Умеренный boost, thermal caps будет управлять max
    # ═══════════════════════════════════════════════════════════════

    # Top-App: Умеренный буст
    echo 5 > /dev/stune/top-app/schedtune.boost
    echo 1 > /dev/stune/top-app/schedtune.prefer_idle

    # Foreground: Без буста
    echo 0 > /dev/stune/foreground/schedtune.boost
    echo 0 > /dev/stune/foreground/schedtune.prefer_idle

    # Background: Минимизируем
    echo 0 > /dev/stune/background/schedtune.boost
    echo 0 > /dev/stune/background/schedtune.prefer_idle

    # ═══════════════════════════════════════════════════════════════
    # 3. CPUSETS (Изоляция ядер)
    # ═══════════════════════════════════════════════════════════════

    echo 0-2 > /dev/cpuset/background/cpus
    echo 0-3 > /dev/cpuset/system-background/cpus
    echo 0-7 > /dev/cpuset/top-app/cpus
    echo 0-3 > /dev/cpuset/restricted/cpus

    # ═══════════════════════════════════════════════════════════════
    # 4. WALT/SCHEDULER TUNING
    # ═══════════════════════════════════════════════════════════════

    echo 1 > /proc/sys/kernel/sched_walt_rotate_big_tasks 2>/dev/null

    # ★ ИЗМЕНЕНО: Более агрессивная миграция на big для отзывчивости
    echo 80 > /proc/sys/kernel/sched_upmigrate 2>/dev/null
    echo 60 > /proc/sys/kernel/sched_downmigrate 2>/dev/null
    echo 85 > /proc/sys/kernel/sched_group_upmigrate 2>/dev/null
    echo 65 > /proc/sys/kernel/sched_group_downmigrate 2>/dev/null
}

function configure_storage_io() {
    # ═══════════════════════════════════════════════════════════════
    # I/O настройки (без ZRAM — он в init.qcom.rc)
    # ═══════════════════════════════════════════════════════════════

    echo 128 > /sys/block/mmcblk0/queue/read_ahead_kb 2>/dev/null
    echo 128 > /sys/block/mmcblk0/bdi/read_ahead_kb 2>/dev/null
    echo "mq-deadline" > /sys/block/mmcblk0/queue/scheduler 2>/dev/null
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN EXECUTION
# ═══════════════════════════════════════════════════════════════════════════

target=`getprop ro.board.platform`

case "$target" in
    "sdm660" | "sdm636")
        # 1. IRQ affinity
        echo f > /proc/irq/default_smp_affinity

        # 2. Core Control
        if [ -d /sys/devices/system/cpu/cpu4/core_ctl ]; then
            echo 2 > /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
            echo 4 > /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
            echo 60 > /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
            echo 30 > /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
            echo 100 > /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
            echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster
        fi

        # 3. CPU и Scheduler
        sdm660_sched_schedutil_dcvs

        # 4. Storage I/O
        configure_storage_io

        # 5. Bus DCVS (DDR bandwidth)
        for device in /sys/devices/platform/soc; do
            for cpubw in $device/*cpu-cpu-ddr-bw/devfreq/*cpu-cpu-ddr-bw; do
                if [ -d "$cpubw" ]; then
                    echo "bw_hwmon" > $cpubw/governor 2>/dev/null
                    echo 50 > $cpubw/polling_interval 2>/dev/null
                    echo 762 > $cpubw/min_freq 2>/dev/null
                    echo "762 1571 2086 2929 3879 5163 5931 6881" > $cpubw/bw_hwmon/mbps_zones 2>/dev/null
                    echo 4 > $cpubw/bw_hwmon/sample_ms 2>/dev/null
                    echo 85 > $cpubw/bw_hwmon/io_percent 2>/dev/null
                    echo 100 > $cpubw/bw_hwmon/decay_rate 2>/dev/null
                    echo 50 > $cpubw/bw_hwmon/bw_step 2>/dev/null
                    echo 20 > $cpubw/bw_hwmon/hist_memory 2>/dev/null
                    echo 0 > $cpubw/bw_hwmon/hyst_length 2>/dev/null
                    echo 80 > $cpubw/bw_hwmon/down_thres 2>/dev/null
                    echo 0 > $cpubw/bw_hwmon/guard_band_mbps 2>/dev/null
                    echo 250 > $cpubw/bw_hwmon/up_scale 2>/dev/null
                    echo 1600 > $cpubw/bw_hwmon/idle_mbps 2>/dev/null
                fi
            done
        done

        # 6. CDSP
        start vendor.cdsprpcd
    ;;
esac

# Флаг завершения
setprop vendor.post_boot.parsed 1
