local src = Source {
    name = "mlibc",
    Git ("https://github.com/managarm/mlibc.git", "3769f1a760a695e5276cb1f2dcab1dc9e2c27c2c"),
    patches = {
        "dist/sources/mlibc/patches/0001-port-evalynos.patch",
    }
}

return src
