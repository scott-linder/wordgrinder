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
