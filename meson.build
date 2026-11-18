#
#   Half of this was swooshed from Google and I hope a future collaborator will patch whatever's broken. (if anything is broken at all)
#   Also due to that fact, most comments are just me guessing what it does. (well I Googled some of it so it's probably right)
#   

#   Why are there no multi line comments yet?

project('velocity', 'cpp',
  version : run_command('git', 'describe', '--tags', '--always', check : true).stdout().strip(), # Git version
  license : 'MIT',
  default_options : ['warning_level=2', 'cpp_std=c++17']
)

# Runs the Git CLI to get the commit and branch info
git_commit = run_command('git', 'describe', '--always', '--dirty', '--exclude=*', check : false).stdout().strip()
if git_commit == ''
  git_commit = 'unknown'
endif

git_branch = run_command('git', 'branch', '--show-current', check : false).stdout().strip()
if git_branch == ''
  git_branch = 'detached'
endif

# The encoder needs zstd to compress and json-glib to read the scene files.
zstd_dep = dependency('libzstd')
json_dep = dependency('json-glib-1.0')

# All headers must be placed in `include_directories`.
inc = include_directories('include')

executable('velocity',
  'src/main.cpp',
  'src/cli.cpp',
  'src/cfgen.cpp',
  'src/compiler.cpp',
  'src/framework.cpp',
  'src/project.cpp',
  'src/vrtx_encoder.cpp',
  include_directories : inc,
  dependencies : [zstd_dep, json_dep],

  # Sets the arguments for `cfgen.hpp` (stands for config generator by the way)
  cpp_args : [
    '-DCOMMIT="@0@"'.format(git_commit),
    '-DBRANCH="@0@"'.format(git_branch)
  ],

  link_args : ['-static-libgcc', '-static-libstdc++'],
  install : true
)