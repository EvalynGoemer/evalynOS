local function target_package(package)
    require(string.gsub("dist.target.${pkg}.${pkg}", "${pkg}", package))
end

target_package("mlibc")
target_package("mlibc-headers")
target_package("basic-rt")
target_package("klib")
target_package("meow")
target_package("helloworld")
