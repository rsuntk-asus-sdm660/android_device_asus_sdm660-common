#!/vendor/bin/sh

write() { [ -e "$1" ] && echo "$2" > "$1"; }
write_str() { [ -e "$1" ] && printf '%s' "$2" > "$1"; }

function sdm660_sched_schedutil_dcvs() {
    # Big Cluster (Kryo Gold: CPU 0-3)
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_governor "schedutil"
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/up_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/down_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy0/schedutil/iowait_boost_enable 0
    write /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq 576000
    # Little Cluster (Kryo Silver: CPU 4-7)
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_governor "schedutil"
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/up_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/down_rate_limit_us 0
    write /sys/devices/system/cpu/cpufreq/policy4/schedutil/iowait_boost_enable 0
    write /sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq 825600
}

target=$(getprop ro.board.platform)

case "$target" in
    "sdm660" | "sdm636")
        log -t "$LOGTAG" -p i "Starting post_boot for platform: $target"

        write /proc/irq/default_smp_affinity f

        if [ -d /sys/devices/system/cpu/cpu4/core_ctl ]; then
            write /sys/devices/system/cpu/cpu4/core_ctl/min_cpus 2
            write /sys/devices/system/cpu/cpu4/core_ctl/max_cpus 4
            write /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres 60
            write /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres 30
            write /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms 100
            write /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster 1
        fi

        # Apply CPU profile
        sdm660_sched_schedutil_dcvs

        # Apply DDR BW profile
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

        # Start the CDSP
        start vendor.cdsprpcd
    ;;
esac

setprop vendor.post_boot.parsed 1
