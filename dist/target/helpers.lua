local cross_files <const> = {
    x86_64      = "evalynos-x86_64.cross",
    riscv64     = "evalynos-riscv64.cross",
    loongarch64 = "evalynos-loongarch64.cross",
}

local function getCrossFile()
    return cross_files[chariot.target_arch]
end

return {
    getCrossFile = getCrossFile,
}
