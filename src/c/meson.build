globals = static_library(
  'globals',
  [
    'utils.cc',
    'cmark.cc',
    'filesystem.cc',
    'main.cc',
    'screen.cc',
    'word.cc',
    'zip.cc',
    'globals.h',
    script_table,
  ],
  dependencies : [
    luau_vm_dep,
    cmark_dep,
    minizip_dep,
  ],
)

wordgrinder = executable(
  'wordgrinder',
  [
    'lua.cc',
    'clipboard.cc'
  ],
  dependencies : [
    luau_vm_dep,
    luau_compiler_dep,
    ncurses,
  ],
  link_with : [
    globals,
  ],
  cpp_args : f'-DFILEFORMAT=@fileformat@',
)
