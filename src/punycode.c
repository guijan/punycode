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

/*
 * This punycode implementation features code from punycode-sample.c 2.0.0
 * written by Adam M. Costello. Specifically, it was taken from:
 * http://www.nicemice.net/idn/punycode-spec.gz
 * SHA256: 693158e1c1ada679c4cc9d72f48527a4490bb24ae81c3869a54889a0fdd58b0c
 *
 * I have not actually authored the encoder, only restyled and modified the
 * code. This derivative work is distributed under a different license, which is
 * explicitly allowed by the original's license.
 */

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "punycode.h"

enum {
	/* Punycode params. */
	base		= 36,
	tmin		= 1,
	tmax		= 26,
	skew		= 38,
	damp		= 700,
	initial_bias	= 72,
	initial_n	= 128,
};

static int utf8dec_unsafe(uint_least32_t *, void *);
static unsigned char encode_digit(uint_least32_t);
static uint_least32_t adapt(uint_least32_t, uint_least32_t, int);

/* punyenc: punycode encoder
 * Encodes at most dstlen-1 bytes to _dst, terminating _dst with '\0' if
 * dstlen > 0. Returns the total length of the string it tried to create if the
 * input wasn't too long, (size_t)-1 otherwise.
 *
 * Set _dst to NULL and dstlen to 0 to get the return value without writing
 * anything.
 *
 * No validation is done. Don't pass invalid UTF-8 input to the encoder.
 *
 * This interface is modeled after strlcpy(). The usage is the exact same,
 * except for the possible (size_t)-1 return value which indicates that the
 * input was too large to be encodable.
 */
size_t
punyenc(char *_dst, const char _src[static 1], size_t dstsize)
{
	size_t i;
	uint_least32_t k;
	unsigned char *src = (unsigned char *)_src;
	unsigned char *p;
	unsigned char *dst = (unsigned char *)_dst;
	uint_least32_t h, b;
	uint_least32_t n;
	uint_least32_t delta;
	uint_least32_t bias;
	uint_least32_t m;
	uint_least32_t codepoint;
	uint_least32_t srclen;
	int ret;
	size_t rval = -1;

	/* First, copy the basic chars. */
	n = initial_n;
	srclen = i = 0;
	for (p = src; *p != '\0'; p += ret) {
		ret = utf8dec_unsafe(&codepoint, p);
		srclen++;
		if (codepoint < n) {
			/*
			 * The code reads: If dst is large enough, write the
			 * punycode and calculate its strlen(), otherwise, only
			 * calculate its strlen().
			 *
			 * The following 3 lines of code are used every time we
			 * write to dst.
			 */
			if (i < dstsize)
				dst[i] = *p;
			i++;
		}
	}
	h = b = i;
	if (i > 0) {
		if (i < dstsize)
			dst[i] = '-';
		i++;
	}

	delta = 0;
	bias = initial_bias;
	while (h < srclen) {
		uint_least32_t left, right, result;
		for (m = UINT_LEAST32_MAX, p = src; *p != '\0';) {
			p += utf8dec_unsafe(&codepoint, p);
			if (codepoint >= n && codepoint < m)
				m = codepoint;
		}
		left = m - n;
		right = h + 1;
		result = left * right;
		if (left != 0 && result / left != right)
			goto end; /* Overflow. */
		delta += result;
		n = m;

		for (p = src; *p != '\0';) {
			p += utf8dec_unsafe(&codepoint, p);
			if (codepoint < n && ++delta == 0)
				goto end; /* Overflow. */
			if (codepoint == n) {
				uint_least32_t q, t;
				for (q = delta, k = base;; k += base) {
					t = k <= bias ? tmin :
					    k >= bias + tmax ? tmax : k - bias;
					if (q < t)
						break;
					if (i < dstsize) {
						dst[i] = encode_digit(
						    t + (q - t) % (base - t));
					}
					i++;
					q = (q - t) / (base - t);
				}

				if (i < dstsize)
					dst[i] = encode_digit(q);
				i++;
				bias = adapt(delta, h + 1, h == b);
				delta = 0;
				h++;
			}
		}
		delta++;
		n++;
	}
	rval = i;
end:
	/* Make sure we don't i++ here to mirror strlcpy() behavior. */
	if (i < dstsize)
		dst[i] = '\0';
	else if (dstsize > 0)
		dst[dstsize-1] = '\0';
	return rval;
}

/* utf8dec_unsafe: decode utf8, assuming it is valid.
 *
 * Puts the codepoint in *codepoint.
 * Returns the amount of bytes in the utf-8 encoding.
 */
static int
utf8dec_unsafe(uint_least32_t *codepoint, void *_str)
{
	unsigned char *str = _str;
	int len;
	int i;

	if (*str < 0x80) {
		*codepoint = *str;
		return 1;
	} else if (*str < 0xE0) {
		len = 2;
	} else if (*str < 0xF0) {
		len = 3;
	} else {
		len = 4;
	}

	*codepoint = *str & (0x7F >> len);
	i = 1;
	do {
		/*
		 * Return if we run into a '\0' because the UTF-8 stream is
		 * corrupted. This turns UB (buffer overread) into garbage in,
		 * garbage out.
		 */
		if (str[i] == '\0')
			return i;
		*codepoint <<= 6;
		*codepoint |= str[i] & 0x3F;
	} while (++i < len);
	return len;
}

static unsigned char
encode_digit(uint_least32_t d)
{
	return d + 22 + 75 * (d < 26);
}

static uint_least32_t
adapt(uint_least32_t delta, uint_least32_t numpoints, int firsttime)
{
	uint_least32_t k;

	delta = firsttime ? delta / damp : delta / 2;
	delta += delta / numpoints;

	for (k = 0; delta > ((base - tmin) * tmax) / 2; k += base)
		delta /= base - tmin;

	return k + (base - tmin + 1) * delta / (delta + skew);
}
