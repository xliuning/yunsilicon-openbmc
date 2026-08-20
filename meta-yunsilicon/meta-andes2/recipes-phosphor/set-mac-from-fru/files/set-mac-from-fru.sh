#!/bin/sh

IFACE="eth0"
SERVICE="xyz.openbmc_project.FruDevice"
OBJECT="/xyz/openbmc_project/FruDevice/SOC_Board"
INTERFACE="xyz.openbmc_project.FruDevice"
PROPERTY="CHASSIS_INFO_AM1"

MAX_RETRIES=20
RETRY_INTERVAL=2

echo "[MAC Config] Waiting for FruDevice object $OBJECT..."

# 1. 等待 FruDevice 准备就绪
count=0
MAC=""
while [ $count -lt $MAX_RETRIES ]; do
    RAW_MAC=$(busctl get-property "$SERVICE" "$OBJECT" "$INTERFACE" "$PROPERTY" 2>/dev/null)
    
    if [ $? -eq 0 ] && [ -n "$RAW_MAC" ]; then
        MAC=$(echo "$RAW_MAC" | awk -F '"' '{print $2}')
        if [ -n "$MAC" ] && [ "$MAC" != "00:00:00:00:00:00" ]; then
            echo "[MAC Config] Found MAC address: $MAC"
            break
        fi
    fi

    count=$((count + 1))
    sleep $RETRY_INTERVAL
done

if [ -z "$MAC" ]; then
    echo "[MAC Config] ERROR: Timed out waiting for FRU MAC." >&2
    exit 1
fi

# 2. 获取当前网卡 MAC，判断是否需要更新
CURRENT_MAC=$(cat /sys/class/net/$IFACE/address 2>/dev/null | tr 'a-z' 'A-Z')
TARGET_MAC=$(echo "$MAC" | tr 'a-z' 'A-Z')

if [ "$CURRENT_MAC" = "$TARGET_MAC" ]; then
    echo "[MAC Config] MAC is already set to $TARGET_MAC. Nothing to do."
    exit 0
fi

# 3. 设置网卡 MAC
echo "[MAC Config] Changing MAC on $IFACE from $CURRENT_MAC to $TARGET_MAC..."
ip link set dev "$IFACE" down
ip link set dev "$IFACE" address "$TARGET_MAC"
ip link set dev "$IFACE" up

# 4. 通知/重启 phosphor-network-manager 确保 OpenBMC 认知一致
systemctl restart phosphor-network-manager.service 2>/dev/null || true

echo "[MAC Config] Successfully applied MAC: $TARGET_MAC"
