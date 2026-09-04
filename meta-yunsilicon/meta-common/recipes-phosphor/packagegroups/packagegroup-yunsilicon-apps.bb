SUMMARY = "Yunsilicon BMC application packagegroup"
PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"
PACKAGES = "packagegroup-yunsilicon-apps"

RPROVIDES:packagegroup-yunsilicon-apps += "\
        virtual-obmc-chassis-mgmt \
        virtual-obmc-flash-mgmt \
        virtual-obmc-system-mgmt \
        "

RDEPENDS:${PN} = "\
        phosphor-state-manager \
        phosphor-fan-presence-config \
        phosphor-ipmi-flash \
        phosphor-ipmi-ipmb \
        entity-manager \
        mctp \
        pldm \
        phosphor-ipmi-host \
        bmcweb \
        dbus-sensors \
        "
