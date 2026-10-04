local helpers <const> = require("dist.target.helpers")
local klib <const> = require("dist.target.klib.klib")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

Package {
    name = "helloworld",
    version = "0.0git",
    revision = 1,
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, klib,
        crossfiles = crossfiles,
        _ = Source { Local("dist/sources/helloworld") }
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
