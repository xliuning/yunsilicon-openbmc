#!/bin/sh

SEL_MTD=/dev/mtd/sel


# SEL failure should never block BMC boot

if [ ! -e "${SEL_MTD}" ]; then
    echo "SEL mtd device not found"
    exit 0
fi


# Already exists
if ubinfo -a 2>/dev/null | grep -q "Volume name: sel"
then
    echo "SEL UBI volume already exists"
    exit 0
fi


echo "Attach SEL UBI"

if ! ubiattach -p ${SEL_MTD}; then

    echo "ubiattach failed, try format"

    # initialize empty UBI area
    if ! ubiformat ${SEL_MTD} -y; then
        echo "SEL ubiformat failed"
        exit 0
    fi


    if ! ubiattach -p ${SEL_MTD}; then
        echo "SEL attach failed after format"
        exit 0
    fi

fi


sleep 1


if ! ubinfo -a 2>/dev/null | grep -q "Volume name: sel"
then

    echo "Create SEL volume"

    if ! ubimkvol /dev/ubi0 \
            -N sel \
            -m
    then
        echo "SEL volume create failed"
        exit 0
    fi

fi


exit 0
