OBMC_IMAGE_EXTRA_INSTALL:append:andes2 = " \
    phosphor-virtual-sensor \
    switch-util \
    set-mac-from-fru \
    curl \
    ipmitool \
    net-tools \
"
IMAGE_INSTALL:append = " \
    phosphor-state-manager-host \
    phosphor-state-manager-chassis \
    phosphor-hwmon \
"
