# Copyright (c) 2023 Guilherme Janczak <guilherme.janczak@yandex.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

project('punycode', 'c', version: '0.1', license: 'ISC',
        meson_version: '>= 0.58.0',
        default_options : ['c_std=c99', 'warning_level=2', 'b_lto=true'])

# Library
#

add_global_arguments('-D_GNU_SOURCE', language: 'c')

libpunycode = library('punycode', 'src/punycode.c',
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

# Command-line utility and tests.
#

cc = meson.get_compiler('c')

funcs = {
  'err': '#include <err.h>',
  'errx': '#include <err.h>',
  'warnx': '#include <err.h>',
}
libbsd_dep = []
foreach func, header : funcs
  if not cc.has_function(func, prefix: header)
    libbsd_dep = dependency('libbsd-overlay')
    break
  endif
endforeach

if get_option('utility')
  pkgpaths_dict = {
    'openbsd': ['/usr/local/lib', '/usr/local/include'],
    'netbsd': ['/usr/pkg/lib', '/usr/pkg/include'],
  }
  pkgpaths_dict += {'freebsd': pkgpaths_dict['openbsd']}

  pkgpaths = get_option('pkg_paths')
  if pkgpaths.length() == 0
    if host_machine.system() in pkgpaths_dict
      pkgpaths = pkgpaths_dict[host_machine.system()]
    else
      pkgpaths = [[], []]
    endif
  endif
  libunistring_dep = declare_dependency(
    dependencies: cc.find_library('unistring', dirs: pkgpaths[0]),
    include_directories: pkgpaths[1]
  )

  utility = executable(
    'punycode', 'src/utility.c',
    dependencies: [libbsd_dep, libpunycode_dep, libunistring_dep],
    install: true
  )
endif

subdir('test')
