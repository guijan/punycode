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
#include <unistd.h>

#include <punycode.h>
#include <unicase.h>
#include <uninorm.h>

static int punyutil(void);

/* punyenc: command line front-end to my punycode encoder.
 *
 * This program reads UTF-8 lines from stdin, punyencodes them, and
 * prints US-ASCII to stdout.
 */
int
main(int argc, char *argv[])
{
	int c;
	enum {UNBUFFERED};
	char *tokens[] = {
		[UNBUFFERED] = "unbuffered",
		NULL
	};
	int ret;
	extern char *optarg;
	char *options, *value;

#if defined(__OpenBSD__)
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");
#endif

	while ((c = getopt(argc, argv, "D:")) != -1) {
		switch (c) {
		case 'D': /* Secret debug options. Not for users. */
			options = optarg;
			while (*options) {
				switch (getsubopt(&options, tokens, &value)) {
				case UNBUFFERED:
					/* Disable buffering to prevent the test
					 * rig from blocking forever as it waits
					 * for a write that never flushes.
					 */
					ret = setvbuf(stdin, NULL, _IONBF, 0);
					if (ret)
						err(1, "setvbuf(stdin)");
					ret = setvbuf(stdout, NULL, _IONBF, 0);
					if (ret)
						err(1, "setvbuf(stdout)");
					if (value) {
						errx(1,
						    "option -D: suboption"
	   					    " 'unbuffered' takes no"
	   					    " argument");
					}
					break;
				case -1:
					errx(1, "option -D: missing or illegal"
					    " suboption");
	   				break;
		   		}
	    		}
			break;
		default:
			exit(1);
		}
	}

	return punyutil();
}

static int
punyutil(void)
{
	ssize_t inlen;
	char *in;
	char *fold;
	char *out;
	size_t insz;
	size_t foldsz;
	size_t outsz;
	int rval = 0;
	void *tmp;
	size_t foldlen;
	size_t outlen;

	in = fold = out = NULL;
	insz = foldsz = outsz = 0;
	for (;;) {
		/* Read a line. */
		if ((inlen = getline(&in, &insz, stdin)) == -1) {
			if (feof(stdin))
				return rval;
			err(1, "getline");
		}

		/*
		 * Canonicalize and fold the case of the line.
		 *
		 * u8_tolower() receives the size of the buffer fold points to
		 * in its last parameter, and returns the length of the string
		 * it created in the same parameter. We keep track of
		 * these separately.
		 */
		foldlen = foldsz;
		tmp = u8_tolower(in, inlen, NULL, UNINORM_NFC, fold, &foldlen);
		if (tmp == NULL)
			err(1, "u8_tolower");
		if (tmp != fold) {
			/*
			 * u8_tolower() allocated a new buffer because fold
			 * wasn't large enough.
			 */
			free(fold);
			fold = tmp;
			/*
			 * The sz of this new buffer matches the len of the
			 * string inside it.
			 */
			foldsz = foldlen;
		}
		/*
		 * unistring braindamage: u8_tolower does not '\0' terminate.
		 * That's okay, we just need to overwrite the newline that we
		 * left at the end of the string with a '\0'.
		 */
		fold[--foldlen] = '\0';


		/* Encode the line. */
		if ((outlen = punyenc(out, fold, outsz)) == (size_t)-1) {
			warnx("%s", "punyenc: irrecoverable encoding error");
			rval = 1;
			continue;
		} else if (outlen >= outsz) {
			/* output wasn't large enough, resize it. */
			outsz = outlen+1;
			if ((tmp = realloc(out, outsz)) == NULL)
				err(1, "realloc");
			out = tmp;

			(void)punyenc(out, fold, outsz);
		}
		/* Use the '\0' terminator's storage to store a newline. */
		out[outlen++] = '\n';

		if (fwrite(out, 1, outlen, stdout) < outlen)
			err(1, "fwrite");
	}
	/* NOTREACHED */
}
