project(
  'wordgrinder',
  'c', 'cpp',
  meson_version : '>= 1.3.0',
  version : '0.9',
  default_options : ['warning_level=1'],
)

ncurses = dependency('ncursesw')

cmake = import('cmake')

luau_opts = cmake.subproject_options()
luau_opts.add_cmake_defines({
  'LUAU_BUILD_CLI': false,
  'LUAU_BUILD_TESTS': false,
  'LUAU_EXTERN_C': true,
})
luau = cmake.subproject('luau', options: luau_opts)
luau_vm_dep = luau.dependency('Luau.VM')
luau_compiler_dep = luau.dependency('Luau.Compiler')
luau_analysis_dep = luau.dependency('Luau.Analysis')

cmark_opts = cmake.subproject_options()
cmark_opts.add_cmake_defines({
  'CMARK_TESTS': false,
  'CMARK_STATIC': true,
  'CMARK_SHARED': false,
})
cmark = cmake.subproject('cmark', options: cmark_opts)
cmark_dep = cmark.dependency('cmark_static')

minizip = cmake.subproject('minizip')
minizip_dep = minizip.dependency('minizip')

fileformat = 8

configure_file(
  input : 'wordgrinder.man',
  output : 'wordgrinder.1',
  configuration : {
    'version' : meson.project_version(),
  }
)

subdir('src/lua/')
subdir('src/c/')
subdir('tools/')
subdir('tests/')
