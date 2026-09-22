FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://override.conf \
"


do_install:append() {

    echo "===== ANDES2 SOL TEST ====="

    install -d \
        ${D}${systemd_system_unitdir}/obmc-console@ttyS2.service.d
    ln -sf ../obmc-console.conf \
        ${D}${sysconfdir}/obmc-console/server.ttyS2.conf

    install -m 0644 \
        ${WORKDIR}/sources-unpack/override.conf \
        ${D}${systemd_system_unitdir}/obmc-console@ttyS2.service.d/override.conf
}
