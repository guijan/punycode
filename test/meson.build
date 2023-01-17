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

if not get_option('tests')
  subdir_done()
endif

test('libpunycode', executable('codec', 'libpunycode.c', 'punytest.c',
                           dependencies: [libbsd_dep, libpunycode_dep]))


if get_option('utility')
  # Test the encoder inside the utility
  test('punycode', executable('utility', 'punycode.c', 'punytest.c',
                             dependencies: [libbsd_dep]),
                  args: punycode_exe)

  # Test the option parsing in the utility.
  test('punycode - extraneous argument', punycode_exe, args: 'extraneous',
       should_fail: true)
  test('punycode - invalid option', punycode_exe, args: '-Z',
       should_fail: true)
  test('punycode - bad suboption 1', punycode_exe, args: '-Dunbuffered=',
       should_fail: true)
  test('punycode - bad suboption 2', punycode_exe, args: '-Dunbuffered=bad',
       should_fail: true)
endif
