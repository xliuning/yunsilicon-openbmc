SUMMARY = "SEL UBI storage init"

LICENSE = "CLOSED"


SRC_URI = " \
    file://sel-ubi-init.sh \
    file://sel-ubi-init.service \
"


S = "${UNPACKDIR}"


inherit systemd


SYSTEMD_SERVICE:${PN} = "sel-ubi-init.service"


do_install() {

    install -d ${D}${sbindir}

    install -m 0755 \
        ${UNPACKDIR}/sel-ubi-init.sh \
        ${D}${sbindir}/sel-ubi-init.sh


    install -d ${D}${systemd_system_unitdir}

    install -m 0644 \
        ${UNPACKDIR}/sel-ubi-init.service \
        ${D}${systemd_system_unitdir}/

}


FILES:${PN} += " \
    ${sbindir}/sel-ubi-init.sh \
    ${systemd_system_unitdir}/sel-ubi-init.service \
"
