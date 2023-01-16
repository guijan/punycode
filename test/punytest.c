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

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "punytest.h"

const struct punytest teststr[] = {
	/*
	 * These test strings are taken from Wikipedia's punycode
	 * article. https://wikipedia.org/wiki/Punycode
	 */
	{"", ""},
	{"a", "a-"},
	{"A", "A-"},
	{"3", "3-"},
	{"-", "--"},
	{"London", "London-"},
	{"Lloyd-Atkinson", "Lloyd-Atkinson-"},
	{"This has spaces", "This has spaces-"},
	{"-> $1.00 <-", "-> $1.00 <--"},
	{"а", "80a"},
	{"ü", "tda"},
	{"α", "mxa"},
	{"例", "fsq"},
	{"😉", "n28h"},
	{"αβγ", "mxacd"},
	{"München", "Mnchen-3ya"},
	{"Mnchen-3ya", "Mnchen-3ya-"},
	{"München-Ost", "Mnchen-Ost-9db"},
	{"Bahnhof München-Ost", "Bahnhof Mnchen-Ost-u6b"},
	{"abæcdöef", "abcdef-qua4k"},
	{"правда", "80aafi6cg"},
	{"ยจฆฟคฏข", "22cdfh1b8fsa"},
	{"도메인", "hq1bm8jm9l"},
	{"ドメイン名例", "eckwd4c7cu47r2wf"},
	{"MajiでKoiする5秒前", "MajiKoi5-783gue6qz075azm5e"},
	{"「bücher」", "bcher-kva8445foa"},

	/*
	 * Witty Portuguese poems taken from
	 * https://archive.ph/OTzxQ
	 * https://profecuqui.blog/2021/02/21/\
	 * portugues-poema-para-mostrar-como-este-idioma\
	 * -pode-ser-complexo-na-acentuacao-das-palavras/
	 *
	 * They feature heavy use of Portuguese accentuation.
	 * I ran them through a web punycode encoder at
	 * https://www.punycoder.com/ to get the result instead of my
	 * own encoder to avoid putting an incorrect encode in the
	 * output.
	 */
	{"Entre doidos e doídos, prefiro não acentuar",
	    "Entre doidos e dodos, prefiro no acentuar-yod92b"},
	{"Quem baba não é a babá", "Quem baba no  a bab-8ub7a8k"},
	{"Será que a romã é de Roma", "Ser que a rom  de Roma-32b2d5i"},
	{"Calça, você bota; bota, você calça",
	    "Cala, voc bota; bota, voc cala-lvc0a2fq"},
	{"Oxítona é proparoxítona", "Oxtona  proparoxtona-h2b6fp"},

	/* Sentinel. */
	{NULL, NULL},
};

const struct punytest_ux teststr_ux[] = {
	/*
	 * This test input and most of the comments in the initializers are
	 * taken from the Punycode RFC's sample strings.
	 */

	/*
	 * The first several examples are all translations of the sentence "Why
	 * can't they just speak in <language>?" (courtesy of Michael Kaplan's
	 * "provincial" page [PROVINCIAL]).  Word breaks and punctuation have
	 * been removed, as is often done in domain names.
	 */

	/* (A) Arabic (Egyptian): */
	{"u+0644 u+064A u+0647 u+0645 u+0627 u+0628 u+062A u+0643 u+0644"
            "u+0645 u+0648 u+0634 u+0639 u+0631 u+0628 u+064A u+061F",
	 "egbpdaj6bu4bxfgehfvwxn"},
	/* (B) Chinese (simplified): */
	{"u+4ED6 u+4EEC u+4E3A u+4EC0 u+4E48 u+4E0D u+8BF4 u+4E2D u+6587",
	 "ihqwcrb4cv8a8dqg056pqjye"},
	/* (C) Chinese (traditional): */
	{"u+4ED6 u+5011 u+7232 u+4EC0 u+9EBD u+4E0D u+8AAA u+4E2D u+6587",
	 "ihqwctvzc91f659drss3x8bo0yb"},
	/* (D) Czech: */
	{"U+0050 u+0072 u+006F u+010D u+0070 u+0072 u+006F u+0073 u+0074"
            "u+011B u+006E u+0065 u+006D u+006C u+0075 u+0076 u+00ED u+010D"
            "u+0065 u+0073 u+006B u+0079",
         "Proprostnemluvesky-uyb24dma41a"},
	/* (E) Hebrew: */
	{"u+05DC u+05DE u+05D4 u+05D4 u+05DD u+05E4 u+05E9 u+05D5 u+05D8"
            "u+05DC u+05D0 u+05DE u+05D3 u+05D1 u+05E8 u+05D9 u+05DD u+05E2"
	    "u+05D1 u+05E8 u+05D9 u+05EA",
	 "4dbcagdahymbxekheh6e0a7fei0b"},
	/* (F) Hindi (Devanagari): */
	{"u+092F u+0939 u+0932 u+094B u+0917 u+0939 u+093F u+0928 u+094D"
            "u+0926 u+0940 u+0915 u+094D u+092F u+094B u+0902 u+0928 u+0939"
            "u+0940 u+0902 u+092C u+094B u+0932 u+0938 u+0915 u+0924 u+0947"
            "u+0939 u+0948 u+0902",
         "i1baa7eci9glrd9b2ae1bj0hfcgg6iyaf8o0a1dig0cd"},
    	/* (G) Japanese (kanji and hiragana): */
	{"u+306A u+305C u+307F u+3093 u+306A u+65E5 u+672C u+8A9E u+3092"
            "u+8A71 u+3057 u+3066 u+304F u+308C u+306A u+3044 u+306E u+304B",
         "n8jok5ay5dzabd5bym9f0cm5685rrjetr6pdxa"},
	/* (H) Korean (Hangul syllables): */
	{"u+C138 u+ACC4 u+C758 u+BAA8 u+B4E0 u+C0AC u+B78C u+B4E4 u+C774"
            "u+D55C u+AD6D u+C5B4 u+B97C u+C774 u+D574 u+D55C u+B2E4 u+BA74"
            "u+C5BC u+B9C8 u+B098 u+C88B u+C744 u+AE4C",
         "989aomsvi5e83db1d2a355cv1e0vak1dwrv93d5xbh15a0dt30a5j"
            "psd879ccm6fea98c"},
	/* (I) Russian (Cyrillic): */
	{"U+043F u+043E u+0447 u+0435 u+043C u+0443 u+0436 u+0435 u+043E"
            "u+043D u+0438 u+043D u+0435 u+0433 u+043E u+0432 u+043E u+0440"
            "u+044F u+0442 u+043F u+043E u+0440 u+0443 u+0441 u+0441 u+043A"
            "u+0438",
         "b1abfaaepdrnnbgefbaDotcwatmq2g4l"},
	/* (J) Spanish: */
	{"U+0050 u+006F u+0072 u+0071 u+0075 u+00E9 u+006E u+006F u+0070"
            "u+0075 u+0065 u+0064 u+0065 u+006E u+0073 u+0069 u+006D u+0070"
            "u+006C u+0065 u+006D u+0065 u+006E u+0074 u+0065 u+0068 u+0061"
            "u+0062 u+006C u+0061 u+0072 u+0065 u+006E U+0045 u+0073 u+0070"
            "u+0061 u+00F1 u+006F u+006C",
         "PorqunopuedensimplementehablarenEspaol-fmd56a"},
	/* (K) Vietnamese: */
	{"U+0054 u+1EA1 u+0069 u+0073 u+0061 u+006F u+0068 u+1ECD u+006B"
            "u+0068 u+00F4 u+006E u+0067 u+0074 u+0068 u+1EC3 u+0063 u+0068"
            "u+1EC9 u+006E u+00F3 u+0069 u+0074 u+0069 u+1EBF u+006E u+0067"
            "U+0056 u+0069 u+1EC7 u+0074",
         "TisaohkhngthchnitingVit-kjcr8268qyxafd2f1b9g"},

	/*
	 * The next several examples are all names of Japanese music artists,
         * song titles, and TV programs, just because the author happens to
         * have them handy (but Japanese is useful for providing examples
         * of single-row text, two-row text, ideographic text, and various
         * mixtures thereof).
	 */

	/* (L) 3<nen>B<gumi><kinpachi><sensei> */
	{"u+0033 u+5E74 U+0042 u+7D44 u+91D1 u+516B u+5148 u+751F",
         "3B-ww4c5e180e575a65lsy2b"},
	/* (M) <amuro><namie>-with-SUPER-MONKEYS */
	{"u+5B89 u+5BA4 u+5948 u+7F8E u+6075 u+002D u+0077 u+0069 u+0074"
            "u+0068 u+002D U+0053 U+0055 U+0050 U+0045 U+0052 u+002D U+004D"
            "U+004F U+004E U+004B U+0045 U+0059 U+0053",
         "-with-SUPER-MONKEYS-pc58ag80a8qai00g7n9n"},
	/* (N) Hello-Another-Way-<sorezore><no><basho> */
	{"U+0048 u+0065 u+006C u+006C u+006F u+002D U+0041 u+006E u+006F"
            "u+0074 u+0068 u+0065 u+0072 u+002D U+0057 u+0061 u+0079 u+002D"
            "u+305D u+308C u+305E u+308C u+306E u+5834 u+6240",
         "Hello-Another-Way--fc4qua05auwb3674vfr0b"},
	/* (O) <hitotsu><yane><no><shita>2 */
	{"u+3072 u+3068 u+3064 u+5C4B u+6839 u+306E u+4E0B u+0032",
         "2-u9tlzr9756bt3uc0v"},
	/* (P) Maji<de>Koi<suru>5<byou><mae> */
	{"U+004D u+0061 u+006A u+0069 u+3067 U+004B u+006F u+0069 u+3059"
            "u+308B u+0035 u+79D2 u+524D",
         "MajiKoi5-783gue6qz075azm5e"},
	/* (Q) <pafii>de<runba> */
	{"u+30D1 u+30D5 u+30A3 u+30FC u+0064 u+0065 u+30EB u+30F3 u+30D0",
         "de-jg4avhby1noc0d"},
	/* (R) <sono><supiido><de> */
	{"u+305D u+306E u+30B9 u+30D4 u+30FC u+30C9 u+3067",
         "d9juau41awczczp"},

	/*
	 * The last example is an ASCII string that breaks the existing rules
	 * for host name labels.  (It is not a realistic example for IDNA,
	 * because IDNA never encodes pure ASCII labels.)
	 */

	/* (S) -> $1.00 <- */
	{"u+002D u+003E u+0020 u+0024 u+0031 u+002E u+0030 u+0030 u+0020"
            "u+003C u+002D",
         "-> $1.00 <--"},

	/* Sentinel. */
	{NULL, NULL},
};

/* uxtostr: convert u+nnnn notation into an utf-8 C string
 *
 * The input looks like the input_ux strings in struct punytest_ux.
 * The usage is the exact same as strlcpy(), except we deal with invalid input
 * by terminating the program.
 *
 * Tokens are separated by 0 or more spaces, they start with a lower or upper
 * case 'U' which signals that the decoded output should be forced into lower or
 * upper case (like the 'U') after deserialization. After that, we have a '+'
 * character which has no meaning, and after it is a sequence of 4 to 6 case
 * insensitive hexadecimal digits which represent a 32-bit integer.
 *
 * Examples:
 *
 * input: u+003B
 * output: ';'
 * Notice that 0x3B only needs 2 hexadecimal digits, so it's padded with zeroes
 * to fit the minimum of 4.
 *
 * input: u+003b
 * output: ';'
 * This is functionally the same as the previous example. Notice that the case
 * of the last hex digit in the input is meaningless.
 *
 * input: U+0061
 * output: 'A'
 * Note that 0x61 means 'a' in ASCII, but it's forced into upper case because
 * the U is uppercase.
 *
 * input: U+00CCCC
 * output: '𐳌'
 * Note that this one has 6 digits.
 */
size_t
uxtostr(char *dst, const char src[static 1], size_t dstsize)
{
	size_t i;
	int upper;
	unsigned long codepoint;
	size_t j;
	char *p;
	static const char *const hex = "0123456789abcdef";
	enum {UX_MIN_HEXDIG = 4, UX_MAX_HEXDIG = 6};
	char utf8c[MB_CUR_MAX];
	mbstate_t ps = {0};
	size_t ret;
	unsigned long wmax;
	const char *osrc = src;

	i = 0;
	while (*src) {
		/* Whitespace is not meaningful, skip it. */
		if (isspace((unsigned char)*src)) {
			src++;
			continue;
		}

		upper = *src == 'U';
		/* Either we have a codepoint or we have an error. */
		if (tolower((unsigned char)*src) == 'u' && *++src == '+') {
			src++;
			codepoint = 0;
			/* Deserialize the hex number. */
			for (j = 0; j < UX_MAX_HEXDIG; j++) {
				p = strchr(hex, tolower((unsigned char)*src));
				if (p == NULL || *p == '\0')
					break;
				src++;
				codepoint <<= 4;
				codepoint |= (p-hex);
			}
			/* Is the hex number too short or long? */
			if (j < UX_MIN_HEXDIG || j > UX_MAX_HEXDIG)
				goto syntaxerr;

			/* codepoint could be too large for Windows. */
			p = NULL;
			if (codepoint > WINT_MAX) {
				p = "WINT_MAX";
				wmax = WINT_MAX;
			} else if (codepoint > WCHAR_MAX) {
				p = "WCHAR_MAX";
				wmax = WCHAR_MAX;
			}
			if (p) {
				errx(1, "codepoint %s+%04lu"
				    " larger than %s (%lu)",
				    upper ? "U" : "u", codepoint, p, wmax);
			}

			/* XXX: Windows' wctype.h can't handle characters
			 * outside the BMP. Also, case folding correctly is
			 * impossible in Standard C for multiple reasons. For
			 * instance, 'ß' should become "SS" when uppercased and
			 * that's impossible with the C APIs. However, none of
			 * our examples need this at the moment, and if they do,
			 * a test failure will tell us.
			 */
			if (upper)
				codepoint = towupper(codepoint);
			else
				codepoint = towlower(codepoint);
		} else {
	 		goto syntaxerr;
		}

		/* Encode and write the utf-8 character. */
		ret = wcrtomb(utf8c, codepoint, &ps);
		if (ret == (size_t)-1)
			err(1, "wcrtomb: %s+%04lx", upper ? "U" : "u",
			    codepoint);
		if (i+ret <= dstsize)
			memcpy(dst+i, utf8c, ret);
		i += ret;
	}

	if (i < dstsize)
		dst[i] = '\0';
	else if (dstsize > 0)
		dst[dstsize-1] = '\0';

	return i;

syntaxerr:
	errx(1, "invalid u+nnnn notation in '%s' at index %zu",
	    osrc, (size_t)(src-osrc));
}
