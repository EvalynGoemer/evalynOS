local crossfiles = Source {
    name = "crossfiles",
    Local("extras/crossfiles"),
    prepare = string.format ([[
        PREFIX="%s"
        sed -i "s|\${SYSROOT_DIR}|${SYSROOT_DIR}|g" ./*
        sed -i "s|\${PREFIX}|${PREFIX}|g" ./*
    ]], chariot.target_prefix)
}

return crossfiles
