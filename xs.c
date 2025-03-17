#define VERSION "xs 2025-03-17 https://github.com/danielsource/x.git"
#define USAGE "usage: xs HEX_OCTETS [< FILE]\n"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32 /* for _setmode */
#include <io.h>
#include <fcntl.h>
#endif

#define READCHUNK 8192

static unsigned long readbin(unsigned char **out, FILE *f)
{
	unsigned long sz = 0, n;
	unsigned char *buf;

	*out = NULL;
	do {
		if (!(buf = realloc(*out, sz+READCHUNK))) {
			if (*out) {
				free(*out);
				*out = NULL;
			}
			return 0;
		}
		*out = buf;

		n = fread(buf+sz, 1, READCHUNK, f);
		if (ferror(f)) {
			free(buf);
			*out = NULL;
			return 0;
		} else if (!n)
			break;

		sz += n;
	} while (!feof(f));

	return sz;
}

#define OFFSETSTEP 16

static int printhexmatch(
		const unsigned char *data,
		const unsigned char *pat,
		unsigned long datalen,
		unsigned long patlen)
{
	const unsigned char *dataorig, *d, *p;
	unsigned long off = 0;
	int match = 0;

	dataorig = data;
	d = data;
	p = pat;

	while (patlen <= datalen) {
		while (*p == *d) {
			if (p-pat == patlen-1) {
				++match;
				data += patlen-1;
				datalen -= patlen-1;

				off = data-dataorig - (patlen-1);
				fprintf(stdout, "%08lx %lx\n",
					off/OFFSETSTEP * OFFSETSTEP,
					off % OFFSETSTEP);
				break;
			}
			++d;
			++p;
		}
		d = ++data;
		--datalen;
		p = pat;
	}

	return match;
}

int main(int argc, char *argv[])
{
	enum { ErrNone, ErrNoMatch, ErrBadArg, ErrIO };

	unsigned long i, l, buflen, patternlen;
	char c;
	unsigned char *buf, *pattern;
	int ret;

#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
#endif

	if (argc != 2) {
		fputs(USAGE, stderr);
		return ErrBadArg;
	}

	if (argv[1][0] == '-') {
		if (argv[1][1] == 'v' && argv[1][2] == '\0') {
			puts(VERSION);
			return ErrNone;
		} else {
			fputs(USAGE, stderr);
			return ErrBadArg;
		}
	}

	for (l = 0; (c = argv[1][l]) != '\0'; ++l) {
		switch (c) {
		case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
			c += 32;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
		case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
			break;
		default:
			return ErrBadArg;
		}
	}
	if (!l || l % 2 != 0)
		return ErrBadArg;
	patternlen = l/2;

	if (!(pattern = malloc(patternlen))) {
		perror("xs");
		return ErrIO;
	}

	buflen = readbin(&buf, stdin);
	if (!buf) {
		perror("xs");
		free(pattern);
		return ErrIO;
	} else if (buflen < patternlen) {
		if (buf)
			free(buf);
		free(pattern);
		return ErrBadArg;
	}

	i = patternlen-1;
	do {
		switch (argv[1][l-2]) {
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
			pattern[i] = (argv[1][l-2] - '0') << 4;
			break;
		case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
			pattern[i] = (argv[1][l-2] - 'a' + 10) << 4;
		}
		switch (argv[1][l-1]) {
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
			pattern[i] += argv[1][l-1] - '0';
			break;
		case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
			pattern[i] += argv[1][l-1] - 'a' + 10;
		}
		--i;
	} while (l -= 2);

	ret = printhexmatch(buf, pattern, buflen, patternlen) > 0 ? ErrNone : ErrNoMatch;
	free(buf);
	free(pattern);
	return ret;
}
