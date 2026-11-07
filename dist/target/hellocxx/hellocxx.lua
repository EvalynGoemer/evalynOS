local helpers <const> = require("dist.target.helpers")
local libcxx <const> = require("dist.target.libcxx.libcxx")

Package {
    name = "hellocxx",
    version = "0.0git",
    revision = 1,
    source = Source { name = "hellocxx", Local("dist/sources/hellocxx") },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, libcxx
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}
