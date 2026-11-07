local src = Source {
    name = "support",
    Local("extras/support"),
    env = {
        PREFIX = chariot.target_prefix
    },
    prepare = [[
        sed -i "s|\${SYSROOT_DIR}|${SYSROOT_DIR}|g" ./*.cross
        sed -i "s|\${PREFIX}|${PREFIX}|g" ./*.cross
    ]]
}

local pkg = Tool {
    name = "support",
    version = "0.0git",
    revision = 1,
    source = src,
    install = [[
        mkdir -p "${INSTALL_DIR}"/usr/local/lib
        install -m 644 ${SOURCE_DIR}/*.cross ${INSTALL_DIR}/usr/local/lib
    ]]
}

return pkg
