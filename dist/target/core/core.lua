local helpers <const> = require("dist.target.helpers")
local libcxx <const> = require("dist.target.libcxx.libcxx")

Package {
    name = "core",
    version = "0.0git",
    revision = 1,
    source = Source { name = "core", Local("dist/sources/core") },
    dependencies = {
        "base-devel", "meson",
        helpers.require_target("klib"),
        helpers.targetTools, libcxx
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
