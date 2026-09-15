FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://peci-cpu.conf \
    file://peci-dimm.conf \
"

SYSTEMD_SERVICE:${PN} += " \
    xyz.openbmc_project.Hwmon-peci-cpu.service \
    xyz.openbmc_project.Hwmon-peci-dimm.service \
"

do_install:append() {
    install -d ${D}${sysconfdir}/default/obmc/hwmon

    install -m 0644 \
        ${UNPACKDIR}/peci-cpu.conf \
        ${D}${sysconfdir}/default/obmc/hwmon/peci-cpu.conf

    install -m 0644 \
        ${UNPACKDIR}/peci-dimm.conf \
        ${D}${sysconfdir}/default/obmc/hwmon/peci-dimm.conf

    install -d ${D}${systemd_system_unitdir}

    install -m 0644 \
        ${UNPACKDIR}/xyz.openbmc_project.Hwmon-peci-cpu.service \
        ${D}${systemd_system_unitdir}/xyz.openbmc_project.Hwmon-peci-cpu.service

    install -m 0644 \
        ${UNPACKDIR}/xyz.openbmc_project.Hwmon-peci-dimm.service \
        ${D}${systemd_system_unitdir}/xyz.openbmc_project.Hwmon-peci-dimm.service
}
