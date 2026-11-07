local helpers <const> = require("dist.target.helpers")
local mlibc <const> = require("dist.target.mlibc.mlibc")

Package {
    name = "helloworld",
    version = "0.0git",
    revision = 1,
    source = Source { name = "helloworld", Local("dist/sources/helloworld") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, mlibc
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
