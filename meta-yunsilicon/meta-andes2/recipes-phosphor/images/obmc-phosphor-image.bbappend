OBMC_IMAGE_EXTRA_INSTALL:append:andes2 = " \
    curl \
    ipmitool \
    net-tools \
    switch-util \
    set-mac-from-fru \
"
IMAGE_INSTALL:append = " \
    phosphor-hwmon \
"
OBMC_IMAGE_EXTRA_INSTALL:append:andes2 = " phosphor-sel-logger"
