local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local klib = Package {
    name = "klib",
    version = "0.0git",
    revision = 1,
    source = Source { name = "klib", Local("dist/sources/klib") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        crossfiles,
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

return klib
