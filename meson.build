project('punycode', 'c', version : '0.1', license : 'ISC',
        default_options : ['c_std=c99', 'warning_level=3', 'b_lto=true'])

# Library
#

libpunycode = library('libpunycode', 'src/punycode.c',
                      install: true)
incdir = include_directories('src')
libpunycode_dep = declare_dependency(link_with: libpunycode,
                                     include_directories: incdir)

pkg = import('pkgconfig')
pkg.generate(libpunycode,
             name: 'libpunycode',
             description: 'Punycode encoder and decoder',
             filebase: 'libpunycode',
             url: 'https://github.com/guijan/punycode')

funcs = {
  'err': '#include <err.h>',
  'warnx': '#include <err.h>',
  'getline': '#include <stdio.h>', # XXX: add getline() to libobsd
}


# Command-line utility and tests.
#

cc = meson.get_compiler('c')
deps = [libpunycode_dep]
foreach func, header : funcs
  if not cc.has_function(func, prefix: header)
    deps += dependency('libbsd-overlay')
    break
  endif
endforeach

if get_option('utility')
  executable('punycode',
             'src/utility.c',
             dependencies: deps,
             install: true)
endif

subdir('test')
