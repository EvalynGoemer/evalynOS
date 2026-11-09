project(
  'core', 'cpp',
  default_options: [
    'cpp_std=c++26',
    'warning_level=2',
    'optimization=3',
    'b_lto=true',
  ],
)

add_project_link_arguments('-static', language: 'cpp')

sources = files(
  'main.cpp',
  'posix/spawn.cpp',
  'vfs/vfs.cpp',
  'vfs/helpers.cpp',
  'vfs/tmpfs.cpp',
  'vfs/ustar.cpp',
  'vfs/stubs.cpp',
)

klib_dep = dependency('klib')

executable(
  'core.elf',
  sources,
  dependencies: klib_dep,
  install: true,
)
