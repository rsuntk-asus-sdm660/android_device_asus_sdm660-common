#!/system/bin/sh

PREFIX="zram_vm:"

RAM_KB=0
while read -r key val unit; do
    if [ "$key" = "MemTotal:" ]; then
        RAM_KB=$val
        break
    fi
done < /proc/meminfo

RAM_MB=$((RAM_KB / 1024))

if [ "$RAM_MB" -ge 3400 ]; then
    SWAPPINESS=60
else
    SWAPPINESS=100
fi

if [ "$RAM_MB" -ge 5000 ]; then
    EXTRA_FREE=8192
elif [ "$RAM_MB" -ge 3400 ]; then
    EXTRA_FREE=6144
else
    EXTRA_FREE=4096
fi

echo "$SWAPPINESS" > /proc/sys/vm/swappiness 2>/dev/null
echo "$EXTRA_FREE" > /proc/sys/vm/extra_free_kbytes 2>/dev/null
echo "$PREFIX Memory profile applied: swappiness: ${SWAPPINESS}, extra_free: ${EXTRA_FREE}kb" > /dev/kmsg
