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
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>

#include <punycode.h>

#include "punytest.h"

static void punytest(const char *, const char *);
static void mbstowlower(char [static 1]);

int
main(void)
{
	char foldedin[PUNYBUFSZ];
	int i;
	char *in;
	size_t ret;

	setlocale(LC_CTYPE, ".UTF-8");

	for (i = 0; (in = teststr[i].input) != NULL; i++)
		punytest(teststr[i].output, in);

	for (i = 0; (in = teststr_ux[i].input_ux) != NULL; i++) {
		/*
		 * The test strings have mixed case, our encoder expects case
		 * folded input, we need to case fold input.
		 */

		/* We also need to convert the input from u+nnnn notation to
		 * utf-8.
		 */
		ret = uxtostr(foldedin, in, sizeof(foldedin));
		if (ret >= sizeof(foldedin))
			errx(1, "uxtostr: dstsize too small");
		mbstowlower(foldedin);

		punytest(teststr_ux[i].output, foldedin);
	}

	exit(0);
}

/* punytest: feed input to the punycode encoder, compare it to the output */
static void
punytest(const char *output, const char *input)
{
	char buf[PUNYBUFSZ];
	char *errorstr;
	size_t ret;

	ret = punyenc(buf, input, sizeof(buf));
	errorstr = NULL;
	if (ret == (size_t)-1)
		errorstr = "encoder overflow";
	else if (ret >= sizeof(buf))
		errorstr = "encoded result is larger than buf";
	else if (strcasecmp(buf, output))
		errorstr = "encoded result is wrong";

	if (errorstr) {
		printf("punyenc(buf, \"%s\", %zu)\n", input, sizeof(buf));

		/*
		 * Make sure the colon of every line lines up to make
		 * the printout look neat.
		 */
		fputs( "  Return value: ", stdout);
		if (ret == (size_t)-1)
			puts("(size_t)-1");
		else
			printf("%zu\n", ret);
		printf(" Encode result: \"%s\"\n", buf);
		printf("Correct result: \"%s\"\n", output);
		printf("         Error: %s\n", errorstr);
		exit(1);
	}
}

/* mbstowlower: convert multibyte string to lowercase in Standard C. */
static void
mbstowlower(char str[static 1])
{
	mbstate_t mbs = {0};
	mbstate_t ps = {0};
	wchar_t wc;
	size_t i;
	size_t ret;

	for (i = 0; str[i] != '\0'; i += ret) {
		ret = mbrtowc(&wc, str+i, MB_CUR_MAX, &mbs);
		switch (ret) {
		case (size_t)-2:
			/* Can't happen. */
			errx(1, "mbrtowc: s points to an incomplete sequence");
		case (size_t)-1:
			err(1, "mbrtowc");
			break;
		case '\0':
			return;
		}

		wc = towlower(wc);
		if (wcrtomb(str+i, wc, &ps) > ret) {
			/* Let's just assume this doesn't happen for simplicity.
			 */
			errx(1, "wcrtomb: case conversion changed"
			    " the length of the utf-8 encoding");
		}
	}
}
