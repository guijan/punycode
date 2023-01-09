# punycode - punycode encoder
I wrote this program as an exercise because I had never implemented a RFC
and it's simple enough for a first.
If you actually intend to use it, please contact me,
and I will pay more attention to development.

The future directions are to write more tests and a decoder.

## Design
src/punycode.{c,h} contain the encoder and are intended to be usable standalone.
spec/ contains the specification and the reference implementation, useful for
development.

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
