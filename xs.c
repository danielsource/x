#define VERSION "xs 2025-03-13 https://github.com/danielsource/x.git"
#define USAGE "usage: xs HEX_OCTETS\n"

/* XXX: assumes ASCII */
/* XXX: assumes (text mode == binary mode) for stdin */

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
		const unsigned char *hay,
		const unsigned char *n,
		unsigned long haylen,
		unsigned long nlen)
{
	const unsigned char *hayorig, *hp, *np;
	unsigned long off = 0;
	int match = 0;

	hayorig = hay;
	hp = hay;
	np = n;

	while (nlen <= haylen) {
		while (*np == *hp) {
			if (np-n == nlen-1) {
				++match;
				hay += nlen-1;
				haylen -= nlen-1;

				off = hay-hayorig - (nlen-1);
				fprintf(stdout, "%08lx %lx\n",
					off/OFFSETSTEP * OFFSETSTEP,
					off % OFFSETSTEP);
				break;
			}
			++hp;
			++np;
		}
		hp = ++hay;
		--haylen;
		np = n;
	}

	return match;
}

int main(int argc, char *argv[])
{
	enum { ErrNone, ErrNoMatch, ErrBadArg, ErrIO };

	unsigned long i, j, l, buflen, hexlen;
	char c;
	unsigned char *buf, *hex;
	int ret;

#ifdef _WIN32
	/* Windows being annoying: I need to set binary mode for stdin */
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
		case 'A':case 'B':case 'C':case 'D':case 'E': case 'F':
			c += 32;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
		case 'a':case 'b':case 'c':case 'd':case 'e': case 'f':
			break;
		default:
			return ErrBadArg;
		}
	}
	if (!l || l % 2 != 0)
		return ErrBadArg;
	hexlen = l/2;

	if (!(hex = malloc(hexlen))) {
		perror("xs");
		return ErrIO;
	}

	buflen = readbin(&buf, stdin);
	if (!buf) {
		perror("xs");
		free(hex);
		return ErrIO;
	} else if (buflen < l) {
		if (buf)
			free(buf);
		free(hex);
		return ErrBadArg;
	}

	do {
		j = l/2 - 1;
		hex[j] = 0;
		for (i = 0; i < 2; ++i) {
			c = argv[1][l - (i+1)];
			switch (c) {
			case '0':case '1':case '2':case '3':case '4':
			case '5':case '6':case '7':case '8':case '9':
				hex[j] += (c - '0') * (1<<(i<<2));
				break;
			case 'a':case 'b':case 'c':case 'd':case 'e': case 'f':
				hex[j] += (c-'a' + 10) * (1<<(i<<2));
			}
		}
	} while (l -= 2);

	ret = printhexmatch(buf, hex, buflen, hexlen) > 0 ? ErrNone : ErrNoMatch;
	free(buf);
	free(hex);
	return ret;
}
