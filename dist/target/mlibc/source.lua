local src = Source {
    name = "mlibc",
    Git ("https://github.com/managarm/mlibc.git", "91bd66128920e48aaef4272da6dcdfbbe1c185b1"),
    patches = {
        "dist/target/mlibc/patches/0001-port-evalynos.patch",
        "dist/target/mlibc/patches/0002-fix-libcxx.patch",
    }
}

return src
