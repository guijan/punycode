/*
 * Copyright (c) 2023 Guilherme Janczak <guilherme.janczak@yandex.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined(H_PUNYTEST)
#define H_PUNYTEST

#include <stddef.h>

/* Buffers of this size can contain any punycode input or output string used in
 * our tests.
 */
enum {PUNYBUFSZ = 256};

extern const struct punytest {
	/* Input for the punycode encoder, not case folded. */
	char *input;
	/* Expected output of the punycode encoder, not case folded. */
	char *output;
} teststr[];

extern const struct punytest_ux {
	/*
	 * Input for the punycode encoder, using the U+XXXX notation from the
	 * Unicode standard and the Punycode RFC.
	 */
	char *input_ux;
	/* Expected output of the punycode encoder. */
	char *output;
} teststr_ux[];

size_t uxtostr(char *, const char [static 1], size_t);

#endif
