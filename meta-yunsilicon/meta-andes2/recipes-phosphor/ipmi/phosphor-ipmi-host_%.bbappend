FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# Journal-sel support
#PACKAGECONFIG:append = " journal-sel"
#EXTRA_OEMESON:append = " -Djournal-sel=enabled"
#  SDR and sensor support
# in local.conf or phosphor-ipmi-host_%.bbappend
PACKAGECONFIG:append:pn-phosphor-ipmi-host = "   dynamic-storages-only "
PACKAGECONFIG:append = " dynamic-sensors "
SRC_URI += "file://0001-Add-yunsilicon-andes2-power-control.patch \
            file://0002-Change-SEL-default-folder-val-log-ipmi_sel.patch \
            file://0003-Fix-sensor-map-for-cpu0_temp-aggregator-sensor.patch \
            file://0004-Hide-PECI-sensors-CPU_coreX-sensors.patch \
            "
