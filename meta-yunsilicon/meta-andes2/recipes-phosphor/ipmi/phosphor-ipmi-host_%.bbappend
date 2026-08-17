FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#  SDR and sensor support
PACKAGECONFIG:append = " dynamic-sensors "
SRC_URI += "file://0001-Add-yunsilicon-andes2-power-control.patch"
