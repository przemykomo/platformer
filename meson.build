project('platformer', 'cpp', version: '1.0')

if meson.get_compiler('cpp').get_id() == 'msvc'
  extra_args = ['/std:c++latest','/Zc:preprocessor']
elif meson.get_compiler('cpp').get_id() == 'clang'
  extra_args = ['-std=c++20']
else
  extra_args = ['-std=c++20']
endif

cc = meson.get_compiler('cpp')

# Find dependencies
gl_dep = dependency('gl')
m_dep = cc.find_library('m', required : false)
raylib_dep = cc.find_library('raylib', required : false)

cmake = import('cmake')
entt_subproject = cmake.subproject('entt')
entt_dep = entt_subproject.dependency('EnTT')
#raylib_subproject = cmake.subproject('raylib')
#raylib_dep = raylib_subproject.dependency('raylib')

inc_dir = include_directories(
  'src'
  )

# List your source files here
source_cpp = [
  'src/main.cpp',
  'src/MapLevel.cpp'
]

# Build executable
projectname = executable('platformer',
  source_cpp,
  dependencies : [ raylib_dep, gl_dep, m_dep, entt_dep ],
  cpp_args: extra_args)
