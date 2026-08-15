#!/system/bin/sh

PREFIX="zram_userland:"
ZRAM_DEV="/dev/block/zram0"
ZRAM_SYS="/sys/block/zram0"

RAM_KB=0
while read -r key val unit; do
    if [ "$key" = "MemTotal:" ]; then
        RAM_KB=$val
        break
    fi
done < /proc/meminfo

RAM_MB=$((RAM_KB / 1024))

if [ "$RAM_MB" -ge 5000 ]; then
    # 6 GB target
    ZRAM_SIZE="2684354560"      # 2.5 GB
else
    # 3 GB / 4 GB target
    ZRAM_SIZE="2147483648"      # 2 GB
fi
ZRAM_STREAMS="8"
ZRAM_ALGO="lz4"

[ -z "$ZRAM_STREAMS" ] && ZRAM_STREAMS="8"
[ -z "$ZRAM_ALGO" ] && ZRAM_ALGO="lz4"

WAIT=0
while [ ! -e "$ZRAM_SYS/disksize" ] && [ "$WAIT" -lt 30 ]; do
    usleep 100000 2>/dev/null || sleep 1
    WAIT=$((WAIT + 1))
done

if [ ! -e "$ZRAM_SYS/disksize" ]; then
    [ -c /dev/kmsg ] && echo "$PREFIX error: zram0 not found after retry limit!" > /dev/kmsg
    exit 1
fi

swapoff "$ZRAM_DEV" 2>/dev/null
echo 1 > "$ZRAM_SYS/reset" 2>/dev/null
usleep 100000 2>/dev/null

echo "$ZRAM_STREAMS" > "$ZRAM_SYS/max_comp_streams" 2>/dev/null
echo "$ZRAM_ALGO" > "$ZRAM_SYS/comp_algorithm" 2>/dev/null
echo "$ZRAM_SIZE" > "$ZRAM_SYS/disksize"
mkswap "$ZRAM_DEV" >/dev/null 2>&1

# Enable SWAP partition
if swapon -p 32767 "$ZRAM_DEV" 2>/dev/null; then
    setprop vendor.zram.configured 1
    ZRAM_SIZE_MB=$((ZRAM_SIZE / 1024 / 1024))
    [ -c /dev/kmsg ] && echo "$PREFIX active: zram: ${ZRAM_SIZE_MB}MB algo: $ZRAM_ALGO streams: $ZRAM_STREAMS" > /dev/kmsg
else
    [ -c /dev/kmsg ] && echo "$PREFIX failed to activate!" > /dev/kmsg
    exit 1
fi
