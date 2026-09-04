FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# Journal-sel support
#PACKAGECONFIG:append = " journal-sel"
#EXTRA_OEMESON:append = " -Djournal-sel=enabled"
#  SDR and sensor support
# 在 local.conf 或 phosphor-ipmi-host_%.bbappend 中
PACKAGECONFIG:append:pn-phosphor-ipmi-host = "   dynamic-storages-only "
PACKAGECONFIG:append = " dynamic-sensors "
SRC_URI += "file://0001-Add-yunsilicon-andes2-power-control.patch"
