project(
  'hellocxx', 'cpp',
  default_options: [
    'cpp_std=c++26',
    'warning_level=2',
    'optimization=2',
  ],
)

add_project_link_arguments('-static', language: 'cpp')

sources = files(
  'main.cpp',
)

executable(
  'hellocxx.elf',
  sources,
  install: true,
)
