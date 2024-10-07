project('bad-solitaire', 'cpp',
  default_options: ['cpp_std=c++23'])

src = []

subdir('src')
dependencies = [dependency('sdl2'), dependency('SDL2_image'), dependency('SDL2_ttf')]

executable('bs', src, dependencies: dependencies)
