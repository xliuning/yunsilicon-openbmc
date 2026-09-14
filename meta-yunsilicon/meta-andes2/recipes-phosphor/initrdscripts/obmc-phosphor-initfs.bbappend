FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# add flash_eraseall utils
RDEPENDS:${PN} += "mtd-utils"

# add ubifs tool for initramfs config
RDEPENDS:${PN}:append = " mtd-utils-ubifs"

# overwrite origin recipe obmc-init.sh
SRC_URI:append = " \
    file://obmc-init.sh \
"
