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
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/wait.h>

#include "punytest.h"

static int pipechild(int *, int *, const char *);
static void punytestutil(FILE *, FILE *, const char *, const char *);

/* This is a test for the command line utility version of the encoder. */
int
main(int argc, char *argv[])
{
	int cfd_in, cfd_out; /* child fd stdin, stdout */
	FILE *cf_in, *cf_out; /* child FILE stdin, stdout */
	int i;
	int status;
	pid_t pid;
	char buf[PUNYBUFSZ];
	size_t ret;

	setlocale(LC_CTYPE, ".UTF-8");
	if (argc != 2) {
		errx(1, "this test is missing its argument:"
		    " the path to the command line utility");
	}

	/*
	 * Let's create a subprocess with our command line encoder, and hook its
	 * stdin and stdout to FILE structures that are under this test's
	 * programmatic control.
	 */
	if ((pid = pipechild(&cfd_out, &cfd_in, argv[1])) == -1)
		err(1, "pipechild");
	if ((cf_in = fdopen(cfd_in, "w")) == NULL)
		err(1, "fdopen");
	if ((cf_out = fdopen(cfd_out, "r")) == NULL)
		err(1, "fdopen");
	if (setvbuf(cf_in, NULL, _IOLBF, 0))
		err(1, "setvbuf");
	if (setvbuf(cf_out, NULL, _IOLBF, 0))
		err(1, "setvbuf");


	/* Feed input data into the program and check its output. */
	for (i = 0; teststr[i].input != NULL; i++) {
		punytestutil(cf_out, cf_in, teststr[i].output,
		    teststr[i].input);
	}
	for (i = 0; teststr_ux[i].input_ux != NULL; i++) {
		ret = uxtostr(buf, teststr_ux[i].input_ux, sizeof(buf));
		if (ret >= sizeof(buf))
			errx(1, "uxtostr: dstsize too small");
		punytestutil(cf_out, cf_in, teststr_ux[i].output, buf);
	}

	fclose(cf_in);
	fclose(cf_out);

	while (waitpid(pid, &status, 0) == -1) {
		if (errno != EINTR)
			err(1, "waitpid");
	}
	if (WIFSIGNALED(status)) {
		status = WTERMSIG(status);
		errx(1, "child terminated to signal %d: %s", status,
		    strsignal(status));
	}
	if (WIFEXITED(status)) {
		status = WEXITSTATUS(status);
		if (status)
			errx(1, "child returned code %d", status);

	}
	return 0;
}

/* pipechild: fork&exec the program in path, puts its stdout pipe in output and
 * stdin pipe in input.
 *
 * Returns -1 on error.
 */
static pid_t
pipechild(int *output, int *input, const char *path)
{
	int child_stdin[2] = {-1, -1};
	int child_stdout[2] = {-1, -1};
	int pid;

	if (pipe(child_stdin) == -1 || pipe(child_stdout) == -1)
		goto err;

	pid = fork();
	switch(pid) {
	case -1:
		goto err;
	case 0:
		close(child_stdout[0]);
		close(child_stdin[1]);
		if (dup2(child_stdout[1], 1) == -1
		    || dup2(child_stdin[0], 0) == -1
		    || execl(path, "pipechild", "-Dunbuffered", NULL) == -1)
			_exit(1);
		break;
	default:
		close(child_stdout[1]);
		close(child_stdin[0]);
		*output = child_stdout[0];
		*input = child_stdin[1];
		break;
	}

	return pid;
err:
	close(child_stdin[0]);
	close(child_stdin[1]);
	close(child_stdout[0]);
	close(child_stdout[1]);
	return -1;
}

/* punytestutil: feed input to the punycode utility, compare it to output */
static void
punytestutil(FILE *fout, FILE *fin, const char *output, const char *input)
{
	char buf[PUNYBUFSZ];
	char *errorstr;
	char *p;

	if (fprintf(fin, "%s\n", input) < 0)
		err(1, "printf");
	if (fgets(buf, sizeof(buf), fout) == NULL) {
		if (ferror(fout))
			err(1, "fgets");
		errx(1, "fgets: end of file");
	};

	errorstr = NULL;
	p = strchr(buf, '\n');
	if (p == NULL)
		errorstr = "encoded result is larger than buf";
	else
		*p = '\0';
	if (strcasecmp(buf, output))
		errorstr = "encoded result is wrong";

	if (errorstr) {
		printf("echo \"%s\" | punyenc\n", input);

		/*
		 * Make sure the colon of every line lines up to make
		 * the printout look neat.
		 */
		printf(" Encode result: \"%s\"\n", buf);
		printf("Correct result: \"%s\"\n", output);
		printf("         Error: %s\n", errorstr);
		exit(1);
	}
}
