#!/vendor/bin/sh

# ═══════════════════════════════════════════════════════════════════════════
# init.qcom.post_boot.sh for SDM660/SDM636 on Kernel 4.19 (EAS)
# Optimized for Android 15/16 + BFQ + Schedutil + Power HAL Start
# ═══════════════════════════════════════════════════════════════════════════

LOGTAG="post_boot_sdm660"

write() { [ -e "$1" ] && echo "$2" > "$1"; }
write_str() { [ -e "$1" ] && printf '%s' "$2" > "$1"; }

function sdm660_sched_schedutil_dcvs() {
    # ═══════════════════════════════════════════════════════════════
    # 1. CPU FREQUENCY GOVERNOR (SCHEDUTIL)
    # ═══════════════════════════════════════════════════════════════
    # Big Cluster (Kryo Gold: CPU 0-3
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_governor "schedutil"
    # Убираем rate_limit (0), чтобы ядро 4.19 само решало (максимальная отзывчивость)
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/up_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/down_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/iowait_boost_enable 0
    # THERMAL CAPS: Низкий порог для работы термо-профилей
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq 576000

    # Little Cluster (Kryo Silver: CPU 4-7)
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_governor "schedutil"
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/up_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/down_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/iowait_boost_enable 0
    # THERMAL CAPS: Низкий порог для работы термо-профилей
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq 825600

    # ═══════════════════════════════════════════════════════════════
    # 2. SCHEDTUNE (EAS BOOST)
    # ═══════════════════════════════════════════════════════════════
    # Top-App: Легкий буст для плавности UI
    write /dev/stune/top-app/schedtune.boost 10
    write /dev/stune/top-app/schedtune.prefer_idle 1

    # Foreground
    write /dev/stune/foreground/schedtune.boost 0
    write /dev/stune/foreground/schedtune.prefer_idle 0

    # Background
    write /dev/stune/background/schedtune.boost 0
    write /dev/stune/background/schedtune.prefer_idle 0

    # ═══════════════════════════════════════════════════════════════
    # 3. CPUSETS
    # ═══════════════════════════════════════════════════════════════
    # Background: только малые ядра (cpu 4-7)
    write /dev/cpuset/background/cpus 4-7
    write /dev/cpuset/system-background/cpus 4-7
    write /dev/cpuset/restricted/cpus 4-7

    # Top-App: все ядра (0-7)
    write /dev/cpuset/top-app/cpus 0-7

    # ═══════════════════════════════════════════════════════════════
    # 4. WALT TUNING
    # ═══════════════════════════════════════════════════════════════
    # Разрешить ротацию задач на больших ядрах
    write /proc/sys/kernel/sched_walt_rotate_big_tasks 1
    # Пороги миграции (EAS сам управляет, это подсказки)
    write /proc/sys/kernel/sched_upmigrate 95
    write /proc/sys/kernel/sched_downmigrate 85
    write /proc/sys/kernel/sched_group_upmigrate 95
    write /proc/sys/kernel/sched_group_downmigrate 85
}

function configure_storage_io() {
    # ═══════════════════════════════════════════════════════════════
    # I/O TUNING: BFQ for eMMC
    # ═══════════════════════════════════════════════════════════════
    # Internal Storage
    if [ -d /sys/block/mmcblk0 ]; then
        write_str /sys/block/mmcblk0/queue/scheduler "bfq"
        write /sys/block/mmcblk0/queue/read_ahead_kb 128
        # Важно для Flash памяти на BFQ: убирает холостой ход, повышает скорость
        write /sys/block/mmcblk0/queue/iosched/slice_idle 0
        # Включаем low_latency для отзывчивости UI
        write /sys/block/mmcblk0/queue/iosched/low_latency 1
    fi

    # SD Card (если есть)
    if [ -d /sys/block/mmcblk1 ]; then
        write_str /sys/block/mmcblk1/queue/scheduler "bfq"
        write /sys/block/mmcblk1/queue/read_ahead_kb 128
    fi
}

# ═══════════════════════════════════════════════════════════════════════════
# MAIN EXECUTION
# ═══════════════════════════════════════════════════════════════════════════

target=$(getprop ro.board.platform)

case "$target" in
    "sdm660" | "sdm636")
        log -t "$LOGTAG" -p i "Starting post_boot for platform: $target"

        # 1. IRQ affinity (раз msm_irqbalance удален)
        # f = разрешить прерывания на всех ядрах (0-3)
        write /proc/irq/default_smp_affinity f

        # 2. Core Control (Оставляем как было для стабильности)
        if [ -d /sys/devices/system/cpu/cpu4/core_ctl ]; then
            write /sys/devices/system/cpu/cpu4/core_ctl/min_cpus 2
            write /sys/devices/system/cpu/cpu4/core_ctl/max_cpus 4
            write /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres 60
            write /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres 30
            write /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms 100
            write /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster 1
        fi

        # 3. Apply CPU & Scheduler settings
        sdm660_sched_schedutil_dcvs

        # 4. Apply I/O settings
        configure_storage_io

        # 5. Bus DCVS (DDR Bandwidth)
        for device in /sys/devices/platform/soc; do
            for cpubw in $device/*cpu-cpu-ddr-bw/devfreq/*cpu-cpu-ddr-bw; do
                if [ -d "$cpubw" ]; then
                    write_str $cpubw/governor "bw_hwmon"
                    write $cpubw/polling_interval 50
                    write $cpubw/min_freq 762
                    write_str $cpubw/bw_hwmon/mbps_zones "762 1571 2086 2929 3879 5163 5931 6881"
                    write $cpubw/bw_hwmon/sample_ms 4
                    write $cpubw/bw_hwmon/io_percent 85
                    write $cpubw/bw_hwmon/decay_rate 100
                    write $cpubw/bw_hwmon/bw_step 50
                    write $cpubw/bw_hwmon/hist_memory 20
                    write $cpubw/bw_hwmon/hyst_length 0
                    write $cpubw/bw_hwmon/down_thres 80
                    write $cpubw/bw_hwmon/guard_band_mbps 0
                    write $cpubw/bw_hwmon/up_scale 250
                    write $cpubw/bw_hwmon/idle_mbps 1600
                fi
            done
        done

        # 6. CDSP
        start vendor.cdsprpcd
    ;;
esac

# Сигнал о завершении выполнения скрипта
setprop vendor.post_boot.parsed 1

log -t "$LOGTAG" -p i "post_boot complete"
