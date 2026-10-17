project(
  'helloworld', 'c',
  default_options: [
    'c_std=gnu23',
    'warning_level=2',
    'optimization=2',
  ],
)

add_project_link_arguments('-static', language: 'c')

sources = files(
  'main.c',
)

executable(
  'helloworld.elf',
  sources,
  install: true,
)
