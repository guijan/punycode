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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <punycode.h>

/* punyenc: command line front-end to my punycode encoder.
 *
 * This program reads valid utf-8 lines from stdin, punyencodes them, and
 * prints US-ASCII to stdout.
 */
int
main(void)
{
	ssize_t nr;
	char *input;
	size_t inlen;
	char *tmp;
	size_t ret;
	char *output;
	size_t outlen;
	int rval = 0;

	output = input = NULL;
	outlen = inlen = 0;
	for (;;) {
		/* Read a line. */
		if ((nr = getline(&input, &inlen, stdin)) == -1) {
			if (ferror(stdin))
				err(1, "getline");
			exit(rval);
		}
		if ((tmp = memrchr(input, '\n', nr)) != NULL)
			*tmp = '\0';

		/* Encode the line. */
		if ((ret = punyenc(output, input, outlen)) == (size_t)-1) {
			warnx("%s", "punyenc: irrecoverable encoding error ");
			rval = 1;
			continue;
		} else if (ret >= outlen) {
			/* output wasn't large enough, resize it. */
			outlen = ret+1;
			tmp = realloc(output, outlen);
			if (tmp == NULL)
				err(1, "realloc");
			output = tmp;

			(void)punyenc(output, input, outlen);
		}

		/* Output the line. */
		output[ret++] = '\n';
		if (fwrite(output, 1, ret, stdout) < ret)
			err(1, "fwrite");
	}
	/* NOTREACHED */
}
