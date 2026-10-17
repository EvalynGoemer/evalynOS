local helpers <const> = require("dist.target.helpers")
local mlibc <const> = require("dist.target.mlibc.mlibc")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

Package {
    name = "helloworld",
    version = "0.0git",
    revision = 1,
    source = Source { name = "helloworld", Local("dist/sources/helloworld") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, mlibc, crossfiles
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
