project(
  'klib', 'c',
  version: '0.0.0',
  default_options: [
    'warning_level=2',
    'optimization=2',
    'default_library=static',
  ],
)

cc = meson.get_compiler('c')

add_project_arguments(
  cc.get_supported_arguments(
    '-std=gnu23',
    '-ffreestanding',
    '-fno-stack-protector',
    '-fno-pie',
    '-fno-pic',
  ),
  language: 'c',
)

sources = files(
  'src/klib.c',
)

libklib = static_library(
  'klib',
  sources,
  include_directories: include_directories('include'),
  install: true,
)

install_subdir(
  'include/klib',
  install_dir: get_option('includedir'),
  install_mode: 'rw-r--r--',
)

import('pkgconfig').generate(
  libklib,
  name: meson.project_name(),
  version: meson.project_version(),
)
