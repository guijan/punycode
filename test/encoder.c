#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <punycode.h>

struct punycode {
	char *input;
	char *output;
};

int
main(void)
{
	struct punycode test[] = {
		/*
		 * These test strings are taken from Wikipedia's punycode
		 * article. https://wikipedia.org/wiki/Punycode
		 */
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
		 * result.
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
	char buf[256];
	int i;
	size_t ret;
	char *errorstr;

	for (i = 0; test[i].input != NULL; i++) {
		errorstr = NULL;
		ret = punyenc(buf, test[i].input, sizeof(buf));
		if (ret >= sizeof(buf)) {
			errorstr = ret == (size_t)-1 ? "encoder overflow"
			    : "encoded result is larger than buf";
		} else if (strcmp(buf, test[i].output)) {
			errorstr = "encoded result is wrong";
		}
		if (errorstr) {
			printf("punyenc(buf, \"%s\", %zu)\n",
	  		    test[i].input, sizeof(buf));

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
			printf("Correct result: \"%s\"\n", test[i].output);
			printf("         Error: %s\n", errorstr);
			exit(1);
		}
	}
	exit(0);
}
