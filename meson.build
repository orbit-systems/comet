project('comet', 'c', default_options: 'default_library=static')

comet_sources = [
  'src/comet.c',
  'src/cpu.c',
  'src/decode.c',
  'src/dev.c',
  'src/ic.c',
  'src/io.c',
  'src/mmu.c',
  'src/gpu/gpu.c',
  'src/gpu/gpu.h',
]

cc = meson.get_compiler('c')

sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('SDL2_image')
sdl2_main_dep = cc.find_library('SDL2main')
epoxy_dep = dependency('epoxy')
m_dep = cc.find_library('m')

comet_deps = [sdl2_dep, sdl2_image_dep, sdl2_main_dep, epoxy_dep, m_dep]

if host_machine.system() == 'windows'
  ws2_32_dep = cc.find_library('ws2_32')
  comet_deps += ws2_32_dep
endif

executable(
  'comet',
  comet_sources,
  dependencies: comet_deps,
  win_subsystem: 'windows',
  install: true
)
