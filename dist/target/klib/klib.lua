local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local klib = Package {
    name = "klib",
    version = "0.0git",
    revision = 1,
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        crossfiles = crossfiles,
        _ = Source { Local("dist/sources/klib")}
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

return klib
