project(
  'klib', ['c', 'cpp'],
  version: '0.0.0',
  default_options: [
    'c_std=gnu23',
    'cpp_std=c++26',
    'warning_level=2',
    'optimization=3',
    'b_lto=true',
    'default_library=static',
  ],
)

cc = meson.get_compiler('c')

add_project_arguments(
  cc.get_supported_arguments(
    '-ffreestanding',
    '-fno-stack-protector',
    '-fno-pie',
    '-fno-pic',
  ),
  language: 'c',
)

sources = files(
  'src/klib.c',
  'src/cxx/misc.cpp',
  'src/cxx/anon.cpp',
  'src/cxx/pmap.cpp',
  'src/cxx/addrspace.cpp',
  'src/cxx/initramfs.cpp',
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
