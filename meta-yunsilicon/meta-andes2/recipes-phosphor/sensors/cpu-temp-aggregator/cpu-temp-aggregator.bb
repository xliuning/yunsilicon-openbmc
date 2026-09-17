SUMMARY = "CPU temperature aggregator"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"


SRC_URI = " \
    file://cpu-temp-aggregator.cpp \
    file://cpu-temp-aggregator.conf \
    file://cpu-temp-aggregator.service \
"

DEPENDS = "sdbusplus boost systemd"
DEPENDS += "phosphor-dbus-interfaces"
inherit systemd pkgconfig
S = "${UNPACKDIR}"

CXXFLAGS += "-std=c++23"

SYSTEMD_SERVICE:${PN} = "cpu-temp-aggregator.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        ${S}/cpu-temp-aggregator.cpp \
        -o ${B}/cpu-temp-aggregator \
        -lsdbusplus \
        -lphosphor_dbus \
        -lsystemd \
        -lpthread
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/cpu-temp-aggregator ${D}${bindir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/cpu-temp-aggregator.service \
        ${D}${systemd_system_unitdir}/cpu-temp-aggregator.service
}
