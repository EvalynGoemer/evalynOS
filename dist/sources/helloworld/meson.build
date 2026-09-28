project(
  'helloworld', 'c',
  default_options: [
    'warning_level=2',
    'optimization=2',
  ],
)

cc = meson.get_compiler('c')
klib = dependency('klib')

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

add_project_link_arguments('-nostdlib', '-no-pie', '-static', language: 'c')

sources = files(
  'main.c',
)

executable(
  'helloworld.elf',
  sources,
  dependencies: klib,
  install: true,
)
