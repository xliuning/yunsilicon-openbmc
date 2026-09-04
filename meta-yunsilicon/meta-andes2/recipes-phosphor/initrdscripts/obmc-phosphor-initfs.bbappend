FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# 引入 flash_eraseall 工具所在的包
RDEPENDS:${PN} += "mtd-utils"

# 覆盖主配方中的 obmc-init.sh 为我们 meta-andes2 下的自定义脚本
SRC_URI:append = " \
    file://obmc-init.sh \
"
