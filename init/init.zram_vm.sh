#!/vendor/bin/sh

PREFIX="zram_userland:"
RAM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
RAM_MB=$((RAM_KB / 1024))

# Our kernel is optimized enough, so 60% swappiness should be okay.
if [ $RAM_MB -ge 3400 ]; then
    # 4 GB / 6 GB
    SWAPPINESS=60
else
    # 3 GB
    SWAPPINESS=100
fi

# VFS_CACHE_PRESSURE
if [ $RAM_MB -ge 5000 ]; then
    EXTRA_FREE=8192
elif [ $RAM_MB -ge 3400 ]; then
    EXTRA_FREE=6144
else
    EXTRA_FREE=4096
fi

echo "$SWAPPINESS" > /proc/sys/vm/swappiness
echo 0 > /proc/sys/vm/page-cluster
echo 0 > /proc/sys/vm/watermark_boost_factor
echo 50 > /proc/sys/vm/watermark_scale_factor
echo "$EXTRA_FREE" > /proc/sys/vm/extra_free_kbytes

echo 10 > /proc/sys/vm/dirty_background_ratio
echo 20 > /proc/sys/vm/dirty_ratio
echo 3000 > /proc/sys/vm/dirty_expire_centisecs
echo 500 > /proc/sys/vm/dirty_writeback_centisecs
echo 0 > /proc/sys/vm/laptop_mode

echo 1 > /proc/sys/vm/compact_memory 2>/dev/null

setprop vendor.zram.ready 1

echo "$PREFIX Setting memory profile: vfs_pressure: $VFS_PRESSURE extra_free: ${EXTRA_FREE}kb" > /dev/kmsg
