#!/vendor/bin/sh

ZRAM_DEV=/dev/block/zram0
ZRAM_SYS=/sys/block/zram0

RAM_KB=$(grep MemTotal /proc/meminfo | awk '{print $2}')
RAM_MB=$((RAM_KB / 1024))

ZRAM_SIZE=$(getprop vendor.zram.size)
ZRAM_STREAMS=$(getprop vendor.zram.streams)
ZRAM_ALGO=$(getprop vendor.zram.comp_algorithm)

if [ -z "$ZRAM_SIZE" ] || [ "$ZRAM_SIZE" = "0" ]; then
    if [ $RAM_MB -ge 5000 ]; then
        # 6 GB
        ZRAM_SIZE="2684354560"      # 2.5 GB
        ZRAM_STREAMS="8"
        ZRAM_ALGO="lz4"
    else
        # 3/4 GB
        ZRAM_SIZE="2147483648"      # 2 GB
        ZRAM_STREAMS="8"
        ZRAM_ALGO="lz4"
    fi
    echo "ZRAM: Using fallback for ${RAM_MB}MB RAM" > /dev/kmsg
fi

[ -z "$ZRAM_STREAMS" ] && ZRAM_STREAMS="8"
[ -z "$ZRAM_ALGO" ] && ZRAM_ALGO="lz4"

WAIT=0
while [ ! -e "$ZRAM_SYS/disksize" ] && [ $WAIT -lt 30 ]; do
    sleep 0.1
    WAIT=$((WAIT + 1))
done

if [ ! -e "$ZRAM_SYS/disksize" ]; then
    echo "ZRAM: ERROR - zram0 not found after 3s!" > /dev/kmsg
    exit 1
fi

swapoff $ZRAM_DEV 2>/dev/null
echo 1 > ${ZRAM_SYS}/reset 2>/dev/null
sleep 0.1

# 1. Streams (ДО disksize!)
echo "$ZRAM_STREAMS" > ${ZRAM_SYS}/max_comp_streams 2>/dev/null

# 2. Алгоритм
echo "$ZRAM_ALGO" > ${ZRAM_SYS}/comp_algorithm 2>/dev/null

# 3. Размер
echo "$ZRAM_SIZE" > ${ZRAM_SYS}/disksize

# 4. Форматируем
mkswap $ZRAM_DEV

# 5. Включаем swap
swapon -p 32767 $ZRAM_DEV

# ═══════════════════════════════════════════════════════════════
# ПРОВЕРКА И СИГНАЛ ГОТОВНОСТИ
# ═══════════════════════════════════════════════════════════════
if [ $? -eq 0 ]; then
    setprop vendor.zram.configured 1
    ZRAM_SIZE_MB=$((ZRAM_SIZE / 1024 / 1024))
    echo "ZRAM: OK RAM=${RAM_MB}MB zram=${ZRAM_SIZE_MB}MB algo=$ZRAM_ALGO streams=$ZRAM_STREAMS" > /dev/kmsg
else
    echo "ZRAM: FAILED to activate!" > /dev/kmsg
    exit 1
fi
