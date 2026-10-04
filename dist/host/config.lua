local function host_package(package)
    require(string.gsub("dist.host.${pkg}.${pkg}", "${pkg}", package))
end

host_package("xorriso")
host_package("pkgconf")
host_package("limine")
host_package("ovmf2")
host_package("llvm")
host_package("nasm")
host_package("qemu")
