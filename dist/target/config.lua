local function target_package(package)
    require(string.gsub("dist.target.${pkg}.${pkg}", "${pkg}", package))
end

target_package("klib")
target_package("helloworld")
