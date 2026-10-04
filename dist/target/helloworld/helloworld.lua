local helpers <const> = require("dist.target.helpers")
local klib <const> = require("dist.target.klib.klib")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

Package {
    name = "helloworld",
    version = "0.0git",
    revision = 1,
    source = Source { name = "helloworld", Local("dist/sources/helloworld") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, klib,
        crossfiles,
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
