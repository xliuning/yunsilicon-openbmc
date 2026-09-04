SUMMARY = "Configure BMC MAC utility for Yunsilicon Andes2"
DESCRIPTION = "Configure MAC after switch initilization"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "file://set-mac-from-fru.sh \
           file://set-mac-from-fru.service"

S = "${UNPACKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "set-mac-from-fru.service"
do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/set-mac-from-fru.sh ${D}${bindir}/set-mac-from-fru.sh

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/set-mac-from-fru.service ${D}${systemd_system_unitdir}/set-mac-from-fru.service
}

FILES:${PN} = "${bindir}/set-mac-from-fru.sh \
               ${systemd_system_unitdir}/set-mac-from-fru.service"
