local OVMF2_VERSION <const> = "20260821T130731Z"

Tool {
    name = "ovmf2",
    version = OVMF2_VERSION,
    revision = 1,
    source = Source {
        name = "ovmf2",
        Archive (
            string.gsub("https://github.com/osdev0/edk2-ovmf-stable-bins/releases/download/${version}/edk2-ovmf-bins.tar.xz", "${version}", OVMF2_VERSION),
            "a70d594690e838569b3a0d3f1b7ac2c8244f32ef8b5f84a4a677beddec954e1e"
        )
    },
    install = [[
        cp "${SOURCE_DIR}"/ovmf-code-ia32.fd        ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-vars-ia32.fd        ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-code-x86_64.fd      ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-vars-x86_64.fd      ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-code-riscv64.fd     ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-vars-riscv64.fd     ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-code-aarch64.fd     ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-vars-aarch64.fd     ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-code-loongarch64.fd ${INSTALL_DIR}
        cp "${SOURCE_DIR}"/ovmf-vars-loongarch64.fd ${INSTALL_DIR}
    ]]
}
