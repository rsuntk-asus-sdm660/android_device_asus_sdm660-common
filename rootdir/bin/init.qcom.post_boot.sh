#!/vendor/bin/sh

# ═══════════════════════════════════════════════════════════════════════════
# init.qcom.post_boot.sh for SDM660/SDM636 on Kernel 4.19 (EAS)
# Optimized for User Build + BFQ + Schedutil + Thermal Caps
# ═══════════════════════════════════════════════════════════════════════════

function sdm660_sched_schedutil_dcvs() {
    # ═══════════════════════════════════════════════════════════════
    # 1. CPU FREQUENCY GOVERNOR (SCHEDUTIL)
    # ═══════════════════════════════════════════════════════════════

    # Little Cluster (CPU 0-3)
    echo "schedutil" > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
    # Убираем rate_limit (0), чтобы ядро 4.19 само решало (максимальная отзывчивость)
    echo 0 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/up_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/down_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy0/schedutil/iowait_boost_enable

    # ★ THERMAL CAPS: Низкий порог для работы термо-профилей
    echo 576000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq

    # Big Cluster (CPU 4-7)
    echo "schedutil" > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor
    echo 0 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/up_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/down_rate_limit_us
    echo 0 > /sys/devices/system/cpu/cpufreq/policy4/schedutil/iowait_boost_enable

    # ★ THERMAL CAPS: Низкий порог для работы термо-профилей
    echo 825600 > /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq

    # ═══════════════════════════════════════════════════════════════
    # 2. SCHEDTUNE (EAS BOOST)
    # ═══════════════════════════════════════════════════════════════

    # Top-App: Легкий буст для плавности UI
    echo 10 > /dev/stune/top-app/schedtune.boost
    echo 1 > /dev/stune/top-app/schedtune.prefer_idle

    # Foreground
    echo 0 > /dev/stune/foreground/schedtune.boost
    echo 0 > /dev/stune/foreground/schedtune.prefer_idle

    # Background
    echo 0 > /dev/stune/background/schedtune.boost
    echo 0 > /dev/stune/background/schedtune.prefer_idle

    # ═══════════════════════════════════════════════════════════════
    # 3. CPUSETS
    # ═══════════════════════════════════════════════════════════════

    # Background: только малые ядра
    echo 0-3 > /dev/cpuset/background/cpus
    echo 0-3 > /dev/cpuset/system-background/cpus
    # Top-App: все ядра
    echo 0-7 > /dev/cpuset/top-app/cpus
    # Restricted
    echo 0-3 > /dev/cpuset/restricted/cpus

    # ═══════════════════════════════════════════════════════════════
    # 4. WALT TUNING
    # ═══════════════════════════════════════════════════════════════
    
    # Разрешить ротацию задач на больших ядрах
    echo 1 > /proc/sys/kernel/sched_walt_rotate_big_tasks

    # Пороги миграции (EAS сам управляет, это подсказки)
    echo 95 > /proc/sys/kernel/sched_upmigrate
    echo 85 > /proc/sys/kernel/sched_downmigrate
    echo 95 > /proc/sys/kernel/sched_group_upmigrate
    echo 85 > /proc/sys/kernel/sched_group_downmigrate
}

function configure_storage_io() {
    # ═══════════════════════════════════════════════════════════════
    # I/O TUNING: BFQ for eMMC
    # ═══════════════════════════════════════════════════════════════

    # Internal Storage
    if [ -d /sys/block/mmcblk0 ]; then
        echo "bfq" > /sys/block/mmcblk0/queue/scheduler
        echo 128 > /sys/block/mmcblk0/queue/read_ahead_kb
        
        # Важно для Flash памяти на BFQ: убирает холостой ход, повышает скорость
        echo 0 > /sys/block/mmcblk0/queue/iosched/slice_idle 2>/dev/null
        # Включаем low_latency для отзывчивости UI
        echo 1 > /sys/block/mmcblk0/queue/iosched/low_latency 2>/dev/null
    fi

    # SD Card (если есть)
    if [ -d /sys/block/mmcblk1 ]; then
        echo "bfq" > /sys/block/mmcblk1/queue/scheduler
        echo 128 > /sys/block/mmcblk1/queue/read_ahead_kb
    fi
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN EXECUTION
# ═══════════════════════════════════════════════════════════════════════════

target=`getprop ro.board.platform`

case "$target" in
    "sdm660" | "sdm636")
        # 1. IRQ affinity (раз msm_irqbalance удален)
        # f = разрешить прерывания на всех ядрах (0-3)
        echo f > /proc/irq/default_smp_affinity

        # 2. Core Control (Оставляем как было для стабильности)
        if [ -d /sys/devices/system/cpu/cpu4/core_ctl ]; then
            echo 2 > /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
            echo 4 > /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
            echo 60 > /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
            echo 30 > /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
            echo 100 > /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
            echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster
        fi

        # 3. Apply CPU & Scheduler settings
        sdm660_sched_schedutil_dcvs

        # 4. Apply I/O settings
        configure_storage_io

        # 5. Bus DCVS (DDR Bandwidth)
        for device in /sys/devices/platform/soc; do
            for cpubw in $device/*cpu-cpu-ddr-bw/devfreq/*cpu-cpu-ddr-bw; do
                if [ -d "$cpubw" ]; then
                    echo "bw_hwmon" > $cpubw/governor
                    echo 50 > $cpubw/polling_interval
                    echo 762 > $cpubw/min_freq
                    echo "762 1571 2086 2929 3879 5163 5931 6881" > $cpubw/bw_hwmon/mbps_zones
                    echo 4 > $cpubw/bw_hwmon/sample_ms
                    echo 85 > $cpubw/bw_hwmon/io_percent
                    echo 100 > $cpubw/bw_hwmon/decay_rate
                    echo 50 > $cpubw/bw_hwmon/bw_step
                    echo 20 > $cpubw/bw_hwmon/hist_memory
                    echo 0 > $cpubw/bw_hwmon/hyst_length
                    echo 80 > $cpubw/bw_hwmon/down_thres
                    echo 0 > $cpubw/bw_hwmon/guard_band_mbps
                    echo 250 > $cpubw/bw_hwmon/up_scale
                    echo 1600 > $cpubw/bw_hwmon/idle_mbps
                fi
            done
        done

        # 6. CDSP
        start vendor.cdsprpcd
    ;;
esac

setprop vendor.post_boot.parsed 1
