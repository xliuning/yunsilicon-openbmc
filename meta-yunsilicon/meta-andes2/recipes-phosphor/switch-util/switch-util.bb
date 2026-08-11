SUMMARY = "switch util"

LICENSE = "CLOSED"

SRC_URI = "file://switch-util"

S = "${UNPACKDIR}"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/switch-util ${D}${bindir}/switch-util
}

FILES:${PN} += "${bindir}/switch-util"
INSANE_SKIP:${PN} += "already-stripped"
