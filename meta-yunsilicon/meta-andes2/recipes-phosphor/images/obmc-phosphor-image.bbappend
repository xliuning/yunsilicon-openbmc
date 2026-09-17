OBMC_IMAGE_EXTRA_INSTALL:append:andes2 = " \
    curl \
    ipmitool \
    net-tools \
    switch-util \
    set-mac-from-fru \
    mtd-utils \
    mtd-utils-ubifs \
"
IMAGE_INSTALL:append = " \
    phosphor-hwmon \
    cpu-temp-aggregator \
"
OBMC_IMAGE_EXTRA_INSTALL:append:andes2 = " \
	phosphor-sel-logger \
	sel-ubi-init"
EXTRA_USERS_PARAMS:append = " \
    usermod -a -G priv-admin root; \
"
IMAGE_INSTALL:append = " phosphor-user-manager  "
