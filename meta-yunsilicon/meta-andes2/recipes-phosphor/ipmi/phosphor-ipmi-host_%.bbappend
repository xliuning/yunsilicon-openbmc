FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPENDS:append:andes2 = " andes2-yaml-config"

EXTRA_OEMESON:andes2 = " \
    -Dsensor-yaml-gen=${STAGING_DIR_HOST}${datadir}/andes2-yaml-config/ipmi-sensors.yaml \
    -Dinvsensor-yaml-gen=${STAGING_DIR_HOST}${datadir}/andes2-yaml-config/ipmi-inventory-sensors.yaml \
    -Dfru-yaml-gen=${STAGING_DIR_HOST}${datadir}/andes2-yaml-config/ipmi-fru-read.yaml \
    "
SRC_URI += "file://0001-Add-yunsilicon-andes2-power-control.patch"
