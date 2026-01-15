#!/vendor/bin/sh

# ═══════════════════════════════════════════════════════════════
# VM TUNING — UNIVERSAL 3/4/6 GB — ANDROID 13/14
# ═══════════════════════════════════════════════════════════════

# ═══════════════════════════════════════════════════════════════
# ОПРЕДЕЛЕНИЕ RAM
# ═══════════════════════════════════════════════════════════════
RAM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
RAM_MB=$((RAM_KB / 1024))

# ═══════════════════════════════════════════════════════════════
# ЧИТАЕМ SWAPPINESS ИЗ VENDOR_INIT
# ═══════════════════════════════════════════════════════════════
SWAPPINESS=$(getprop vendor.zram.swappiness)

# ═══════════════════════════════════════════════════════════════
# FALLBACK ПО ОБЪЁМУ RAM
# ═══════════════════════════════════════════════════════════════
if [ -z "$SWAPPINESS" ]; then
    if [ $RAM_MB -ge 5500 ]; then
        # 6 GB — можно больше swappiness
        SWAPPINESS=100
    elif [ $RAM_MB -ge 3700 ]; then
        # 4 GB
        SWAPPINESS=85
    else
        # 3 GB — баланс между бенчмарком и повседневкой
        SWAPPINESS=70
    fi
    echo "VM-TUNE: Using fallback swappiness=$SWAPPINESS for ${RAM_MB}MB" > /dev/kmsg
fi

# ═══════════════════════════════════════════════════════════════
# VFS_CACHE_PRESSURE ПО ОБЪЁМУ RAM
# ═══════════════════════════════════════════════════════════════
if [ $RAM_MB -ge 5500 ]; then
    # 6 GB — можно держать больше cache
    VFS_PRESSURE=80
    EXTRA_FREE=8192
elif [ $RAM_MB -ge 3700 ]; then
    # 4 GB
    VFS_PRESSURE=100
    EXTRA_FREE=6144
else
    # 3 GB — агрессивнее освобождаем cache
    VFS_PRESSURE=100
    EXTRA_FREE=4096
fi

# ═══════════════════════════════════════════════════════════════
# ПРИМЕНЯЕМ CORE VM PARAMETERS
# ═══════════════════════════════════════════════════════════════
echo "$SWAPPINESS" > /proc/sys/vm/swappiness
echo 0 > /proc/sys/vm/page-cluster
echo "$VFS_PRESSURE" > /proc/sys/vm/vfs_cache_pressure
echo 0 > /proc/sys/vm/watermark_boost_factor
echo 50 > /proc/sys/vm/watermark_scale_factor
echo "$EXTRA_FREE" > /proc/sys/vm/extra_free_kbytes

# ═══════════════════════════════════════════════════════════════
# DIRTY PAGES — УНИВЕРСАЛЬНЫЕ ЗНАЧЕНИЯ
# ═══════════════════════════════════════════════════════════════
echo 10 > /proc/sys/vm/dirty_background_ratio
echo 20 > /proc/sys/vm/dirty_ratio
echo 3000 > /proc/sys/vm/dirty_expire_centisecs
echo 500 > /proc/sys/vm/dirty_writeback_centisecs
echo 0 > /proc/sys/vm/laptop_mode

# ═══════════════════════════════════════════════════════════════
# MEMORY COMPACTION
# ═══════════════════════════════════════════════════════════════
echo 1 > /proc/sys/vm/compact_memory 2>/dev/null

# ═══════════════════════════════════════════════════════════════
# ГОТОВО
# ═══════════════════════════════════════════════════════════════
setprop vendor.zram.ready 1

echo "VM-TUNE: RAM=${RAM_MB}MB swap=$SWAPPINESS vfs=$VFS_PRESSURE extra=${EXTRA_FREE}kb [OK]" > /dev/kmsg
