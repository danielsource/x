#define VERSION "x 2025-03-12 https://github.com/danielsource/x.git"
#define USAGE "usage: x [-i|-v]\n"

#include <stdio.h>

#ifdef _WIN32 /* for _setmode */
#include <io.h>
#include <fcntl.h>
#endif

#define HEXCOLS 16
#define OFFSET "%08lx:"
#define XX "%02x"
#define XXPAD "  "
#define HEXFMT OFFSET \
	" " XX XX \
	" " XX XX \
	" " XX XX \
	" " XX XX \
	" " XX XX \
	" " XX XX \
	" " XX XX \
	" " XX XX "  "

#define BUFSIZE 8192
static unsigned char buf[BUFSIZE];

#if BUFSIZE % HEXCOLS != 0
#error "BUFSIZE must be multiple of HEXCOLS"
#endif

enum { ErrNone, ErrBadArg, ErrIO };

static void printascii(FILE *out, const unsigned char *data, unsigned long n)
{
	unsigned long i;

	for (i = 0; i < n; ++i)
		fputc(data[i] >= ' ' && data[i] <= '~' ? data[i] : '.', out);
}

static int hexdump(FILE *out, FILE *in)
{
	unsigned long i, n, rem, off = 0;

	do {
		n = fread(buf, 1, BUFSIZE, in);
		if (ferror(in))
			return ErrIO;
		else if (n < HEXCOLS) {
			if (!n)
				break;
			i = 0;
			rem = n;
			goto padding;
		}

		for (i = 0; i < n-HEXCOLS; i += HEXCOLS) {
			fprintf(out, HEXFMT, off,
			        buf   [i], buf [i+1], buf [i+2], buf [i+3],
			        buf [i+4], buf [i+5], buf [i+6], buf [i+7],
			        buf [i+8], buf [i+9], buf[i+10], buf[i+11],
			        buf[i+12], buf[i+13], buf[i+14], buf[i+15]);
			printascii(out, buf+i, HEXCOLS);
			fputc('\n', out);

			off += HEXCOLS;
		}

		rem = n - i;
		if (!rem)
			continue;

padding:
		fprintf(out, OFFSET " ", off);
		if (rem & 1) {
			for (; i < n-1; i += 2)
				fprintf(out, XX XX " ", buf[i], buf[i+1]);
			fprintf(out, XX XXPAD " ", buf[i]);
			for (i = 0; i < HEXCOLS-rem - 1; i += 2)
				fputs(XXPAD XXPAD " ", out);
		} else {
			for (; i < n; i += 2)
				fprintf(out, XX XX " ", buf[i], buf[i+1]);
			for (i = 0; i < HEXCOLS-rem; i += 2)
				fputs(XXPAD XXPAD " ", out);
		}
		fputc(' ', out);
		printascii(out, buf+n - rem, rem);
		fputc('\n', out);

		off += HEXCOLS;
	} while (!feof(in));

	return ErrNone;
}

static int incdump(FILE *out, FILE *in)
{
	unsigned long i, n, sz = 0;

	fputs("unsigned char dump[] = {", out);

	do {
		n = fread(buf, 1, BUFSIZE, in);
		if (ferror(in))
			return ErrIO;
		else if (!n)
			break;

		if (sz > 0)
			fprintf(out, ",0x%02x", buf[0]);
		else
			fprintf(out, "0x%02x", buf[0]);

		for (i = 1; i < n; ++i)
			fprintf(out, ",0x%02x", buf[i]);

		sz += n;
	} while (!feof(in));

	fprintf(out, "};\nunsigned long dumpsize = %lu;\n", sz);

	return ErrNone;
}

/* TODO: implement revdump */
static int revdump(FILE *out, FILE *in)
{
	return ErrNone;
}

int main(int argc, char *argv[])
{
	int err = ErrBadArg;

#ifdef _WIN32
	/* Windows being annoying: I need to set binary mode for stdin */
	_setmode(_fileno(stdin), _O_BINARY);
#endif

	if (argc <= 1) {
		err = hexdump(stdout, stdin);
	} else if (argc == 2 && argv[1][0] == '-') {
		if (argv[1][1] == 'i' && argv[1][2] == '\0')
			err = incdump(stdout, stdin);
		/* else if (argv[1][1] == 'r' && argv[1][2] == '\0')
			err = revdump(stdout, stdin); */
		else if (argv[1][1] == 'v' && argv[1][2] == '\0')
			return puts(VERSION), ErrNone;
	}

	if (err == ErrIO)
		perror("x");

	return err;
}
