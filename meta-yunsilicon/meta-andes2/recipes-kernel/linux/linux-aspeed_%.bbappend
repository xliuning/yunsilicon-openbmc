FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://andes2.cfg \
    file://aspeed-bmc-yunsilicon-andes2.dts \
    file://0001-arm-dts-add-Andes2-board.patch \
"


do_patch:append() {
    echo "Install Andes2 DTS"

    install -d ${S}/arch/arm/boot/dts/aspeed/

    install -m 0644 \
        ${WORKDIR}/sources/aspeed-bmc-yunsilicon-andes2.dts \
        ${S}/arch/arm/boot/dts/aspeed/
}
