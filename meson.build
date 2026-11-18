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

# Runs the Git CLI to get the version and branch info
git_version = run_command('git', 'describe', '--tags', '--always', '--dirty', check : false).stdout().strip()
if git_version == ''
  git_version = 'unknown'
endif

git_branch = run_command('git', 'branch', '--show-current', check : false).stdout().strip()
if git_branch == ''
  git_branch = 'detached'
endif

# All headers must be placed in `include_directories`.
inc = include_directories('include')

executable('velocity',
  'src/main.cpp',
  'src/cli.cpp',
  'src/cfgen.cpp',
  include_directories : inc,

  # Sets the arguments for `cfgen.hpp` (stands for config generator by the way)
  cpp_args : [
    '-DPROJECT_VERSION="@0@"'.format(git_version),
    '-DBRANCH="@0@"'.format(git_branch)
  ],

  install : true
)