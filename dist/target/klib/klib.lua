local helpers <const> = require("dist.target.helpers")

local klib = Package {
    name = "klib",
    version = "0.0git",
    revision = 1,
    source = Source { name = "klib", Local("dist/sources/klib") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        helpers.require_target("mlibc-headers"),
        helpers.require_target("libcxx")
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

return klib
