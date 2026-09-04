# meta-andes2/recipes-phosphor/logging/phosphor-sel-logger_%.bbappend

EXTRA_OEMESON:append = " \
    -Dlog-threshold=true \
    -Dlog-alarm=true \
    -Dsel-delete=true \
"
