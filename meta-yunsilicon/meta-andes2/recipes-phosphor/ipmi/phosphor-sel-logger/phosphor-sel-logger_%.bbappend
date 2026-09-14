FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://phosphor-sel-logger.conf"
do_install:append() {
    install -m 0644 -D ${UNPACKDIR}/phosphor-sel-logger.conf \
        ${D}${sysconfdir}/rsyslog.d/phosphor-sel-logger.conf
}
