# SweetLine Meson syntax sample.
# This file is intentionally self-contained and uses common Meson constructs.
# Official reference: https://mesonbuild.com/

project(
  'sweetline-meson-sample',
  'c',
  version : '1.2.0',
  meson_version : '>=0.63.0',
  default_options : [
    'warning_level=3',
    'c_std=c11',
    'default_library=shared',
    'buildtype=release',
    'b_lto=false',
    'b_ndebug=if-release',
  ],
)

project_name = meson.project_name()
project_version = meson.project_version()
source_root = meson.project_source_root()
build_root = meson.project_build_root()
host_os = host_machine.system()
host_cpu = host_machine.cpu_family()
build_os = build_machine.system()
target_os = target_machine.system()
cross_build = meson.is_cross_build()

cc = meson.get_compiler('c')
fs = import('fs')

warning_flags = []
platform_defs = []
feature_names = ['core', 'cli', 'docs', 'bench']
enabled_modules = []
library_suffix = 'sweetline-@PROJECT_VERSION@'
readme_banner = 'SweetLine @PROJECT_NAME@ @PROJECT_VERSION@'

if cc.get_id() == 'msvc'
  warning_flags += ['/W4', '/permissive-']
elif cc.has_argument('-Wall')
  warning_flags += ['-Wall', '-Wextra', '-Wpedantic']
else
  warning_flags += ['-Wall']
endif

if host_os == 'windows'
  platform_defs += ['-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX']
elif host_os in ['linux', 'darwin']
  platform_defs += ['-D_POSIX_C_SOURCE=200809L']
else
  platform_defs += ['-D_GNU_SOURCE']
endif

if build_os != host_os
  warning_flags += ['-Wno-poison-system-directories']
endif

if target_os == host_os
  message('target system matches host')
else
  message('target system differs from host')
endif

dep_threads = dependency('threads')
dep_zlib = dependency('zlib', required : false)
dep_fmt = dependency('fmt', required : false)
dep_ssl = dependency('openssl', required : false)
math_dep = cc.find_library('m', required : false)

conf = configuration_data()
conf.set('HOST_OS', host_os)
conf.set('HOST_CPU', host_cpu)
conf.set('BUILD_ROOT', build_root)
conf.set_quoted('PROJECT_NAME', project_name)
conf.set_quoted('PROJECT_VERSION', project_version)
conf.set_quoted('SOURCE_ROOT', source_root)
conf.set10('CROSS_BUILD', cross_build)
conf.set10('HAVE_ZLIB', dep_zlib.found())
conf.set10('HAVE_FMT', dep_fmt.found())
conf.set10('HAVE_SSL', dep_ssl.found())
conf.set_quoted('BANNER', readme_banner)
configure_file(
  input : 'config.h.in',
  output : 'config.h',
  configuration : conf,
)

inc = include_directories('include')
srcs = files(
  'src/main.c',
  'src/app.c',
  'src/config.c',
  'src/io.c',
  'src/net.c',
  'src/util.c',
)
headers = files(
  'include/sweetline.h',
  'include/sweetline/config.h',
)

core_args = warning_flags + platform_defs
core_deps = [
  dep_threads,
  dep_zlib,
  dep_fmt,
  dep_ssl,
  math_dep,
]

core_lib = library(
  'sweetline_core',
  srcs,
  include_directories : inc,
  c_args : core_args,
  dependencies : core_deps,
  install : true,
)

core_dep = declare_dependency(
  include_directories : inc,
  link_with : core_lib,
  dependencies : core_deps,
)

app_sources = files(
  'src/demo_main.c',
  'src/demo_render.c',
  'src/demo_assets.c',
)

app = executable(
  'sweetline_demo',
  app_sources,
  include_directories : inc,
  dependencies : [core_dep],
  c_args : core_args,
  install : true,
)

install_headers(
  headers,
  subdir : 'sweetline',
)

install_data(
  'LICENSE',
  install_dir : get_option('datadir') / 'sweetline',
)

message('library suffix: ' + library_suffix)

if dep_zlib.found()
  message('zlib support enabled')
elif dep_fmt.found()
  message('fmt support enabled')
else
  message('optional deps disabled')
endif

if fs.exists('README.md')
  message('documentation exists')
endif

if cross_build
  message('cross build')
else
  message('native build')
endif

foreach feature : feature_names
  if feature == 'bench'
    break
  endif
  message('feature enabled: ' + feature)
endforeach

generated_modules = ['parser', 'runtime', 'cli']
foreach module_name : generated_modules
  if module_name == 'cli'
    enabled_modules += [module_name]
    continue
  endif
  enabled_modules += [module_name]
endforeach

if 'core' in enabled_modules
  message('core module selected')
endif

foreach module_name : enabled_modules
  if module_name == 'runtime'
    continue
  endif
  message('prepared module ' + module_name)
endforeach

test(
  'core-smoke',
  app,
  args : ['--version'],
  timeout : 30,
)

test(
  'core-help',
  app,
  args : ['--help'],
  timeout : 30,
)

if meson.can_run_host_binaries()
  message('host binaries are runnable')
endif

message('project: ' + project_name)
