SUMMARY = "L2 switch initialization utility for Yunsilicon Andes2"
DESCRIPTION = "Pre-network binary that initializes L2 switch hardware before network stack starts"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "file://switch-util \
           file://switch-util.service"

S = "${UNPACKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "switch-util.service"
INSANE_SKIP:${PN} += "already-stripped"
do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/switch-util ${D}${bindir}/switch-util

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/switch-util.service ${D}${systemd_system_unitdir}/switch-util.service
}

FILES:${PN} = "${bindir}/switch-util \
               ${systemd_system_unitdir}/switch-util.service"
