# punycode - punycode encoder
I wrote this program as an exercise because the only other RFC I implemented
before this one was my internet checksum in 8086 assembly which is very simple.
This RFC is more complicated and seemed like a good next step.

If you actually intend to use this program, please contact me,
and I will pay more attention to development.

## Usage
### Building
The library itself has no dependencies.
The command line utility and the tests depend on either of
[libbsd](https://gitlab.freedesktop.org/libbsd/libbsd/) or
[libobsd](https://github.com/guijan/libobsd).
The command line utility additionally depends on
[libunistring](https://www.gnu.org/software/libunistring/).

The compilation process is the usual for Meson:
```console
$ meson setup build
$ meson compile -C build
```

The shell utility and the tests can be disabled:
```console
$ meson setup build -Dutility=false -Dtests=false
```

If you have issues finding libunistring, specify the package library and include
paths manually:
```console
$ meson setup build "-Dpkg_paths=['/usr/local/lib', '/usr/local/include']"
```
Replace /usr/local/lib and /usr/local/include with paths appropriate for your
system.

### Programming
In your build system, tell [pkgconf](https://github.com/pkgconf/pkgconf) to look
for __libpunycode__. This is the only supported method of linking against the
library and providing its include path.

In your C or C++ program, follow [the manual](src/punycode.3).

If you desire more complete example code, read the source of the utility at
[src/utility.c](src/utility.c).

## Development
### API design
The API is designed with the idea that an implementation of a standard should do
what ought to be done within the standard, not literally everything the standard
allows. As such, there is no support for mixed case because it complicates the
encoder and its usage massively and makes no functional difference.

The API uses idiomatic C and targets modern systems and ease of use, so none of
the weirdness that is present in the reference implementation is present in this
implementation. For instance, the reference implementation does not create valid
C strings and requires that the length of the source string be passed explicitly
through an argument, this implementation does what you'd expect for a C function
that takes an input string and writes an output string.

Read the manual and then the source code for in-depth information about the
design and implementation of this punycode codec.

### Source code structure
[src/punycode.c](src/punycode.c) and [src/punycode.h](src/punycode.h) contain
the encoder and are intended to be usable standalone. They are intended to be
maximally compatible, written in pure C99, and make no assumptions about the
underlying machine and operating system, the assumptions we do not make include
but are not limited to the source or execution character sets (although
input/output are UTF-8) or the size of `int`.

[spec/](spec/) contains the specification and the reference implementation,
useful for development.

The other source files target a loosely POSIX 2008+ operating system with UTF-8
support.

[src/](src/) files are written with API simplicity and implementation speed and
robustness in mind.
[test/](test/) files are written with only simplicity in mind.

### Future directions
The future directions are to write more tests and a decoder.
