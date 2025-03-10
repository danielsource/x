#define VERSION "2025-03-10 (https://github.com/danielsource/x.git)"
#define USAGE "usage: x [-i|-v]\n"

#include <limits.h>
#include <stdio.h>

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
#define HEXFMT_MINLEN 51

#define BUFSIZE 8192
static unsigned char buf[BUFSIZE];

#if BUFSIZE % HEXCOLS != 0
#error "BUFSIZE must be multiple of HEXCOLS"
#endif

enum { ErrNone, ErrBadArg, ErrInput, ErrOutput,
       ErrBadFmt, ErrFileBig, ErrNoImpl, ErrCount };

static const char *errmsg[ErrCount] = {
	NULL,
	USAGE,
	NULL,
	NULL,
	"x: invalid format\n",
	"x: file/line is too large\n",
	"x: not implemented\n",
};

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
			return ErrInput;
		else if (ULONG_MAX - off < n)
			return ErrFileBig;
		else if (n < HEXCOLS) {
			if (!n)
				break;
			i = 0;
			rem = n;
			goto padding;
		}

		if (fprintf(out, HEXFMT, off,
		            buf [0], buf [1], buf [2], buf [3],
		            buf [4], buf [5], buf [6], buf [7],
		            buf [8], buf [9], buf[10], buf[11],
		            buf[12], buf[13], buf[14], buf[15]) < HEXFMT_MINLEN)
			return ErrOutput;
		printascii(out, buf, HEXCOLS);
		fputc('\n', out);

		off += HEXCOLS;

		for (i = HEXCOLS; i < n-HEXCOLS; i += HEXCOLS) {
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
	unsigned long i, n, size = 0;

	if (fputs("unsigned char dump[] = {", out) == EOF)
		return ErrOutput;

	do {
		n = fread(buf, 1, BUFSIZE, in);
		if (ferror(in))
			return ErrInput;
		else if (ULONG_MAX - size < n)
			return ErrFileBig;
		else if (!n)
			break;

		if (size > 0)
			fprintf(out, ",0x%02x", buf[0]);
		else
			fprintf(out, "0x%02x", buf[0]);

		for (i = 1; i < n; ++i)
			fprintf(out, ",0x%02x", buf[i]);

		size += n;
	} while (!feof(in));

	fprintf(out, "};\nunsigned long dumpsize = %lu;\n", size);

	return ErrNone;
}

/* TODO: implement revdump */
/* revdump _minimal_ format: <colon><hex-octets><space><space> (offset and ascii is ignored) */
static int revdump(FILE *out, FILE *in)
{
	return ErrNoImpl;
}

int main(int argc, char *argv[])
{
	int err = ErrBadArg;

	if (argc <= 1) {
		err = hexdump(stdout, stdin);
	} else if (argc == 2 && argv[1][0] == '-') {
		if (argv[1][1] == 'i' && argv[1][2] == '\0')
			err = incdump(stdout, stdin);
		else if (argv[1][1] == 'r' && argv[1][2] == '\0')
			err = revdump(stdout, stdin);
		else if (argv[1][1] == 'v' && argv[1][2] == '\0')
			return puts(VERSION), ErrNone;
	}

	if (err == ErrInput || err == ErrOutput)
		perror("x");
	else if (err != ErrNone)
		fputs(errmsg[err], stderr);

	return err;
}
