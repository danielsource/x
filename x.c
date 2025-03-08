#define VERSION "2025-03-08 (https://github.com/danielsource/x.git)"
#define USAGE "usage: x [-i|-r|-v]\n"

#include <limits.h>
#include <stdio.h>

#define HEXCOLS 16

#define BUFSIZE 8192
static unsigned char buf[BUFSIZE];

#if BUFSIZE % HEXCOLS != 0
#error "BUFSIZE must be multiple of HEXCOLS"
#endif

enum {
	ErrNone = 0,
	ErrWrongArg,
	ErrFread,
	ErrPrint,
	ErrFileBig,
	ErrNoImpl,
	ErrCount
};

static int printascii(FILE *out, const unsigned char *data, unsigned long n)
{
	unsigned long i;
	int nc = 0;

	for (i = 0; i < n; ++i) {
		if (data[i] >= ' ' && data[i] <= '~') {
			if (fputc(data[i], out) == EOF)
				return EOF;
		} else {
			if (fputc('.', out) == EOF)
				return EOF;
		}
		++nc;
	}

	return nc;
}

static int hexdump(FILE *out, FILE *in)
{
	unsigned long i, j, n, rem, off = 0;

	clearerr(in);
	for (;;) {
		if (!(n = fread(buf, 1, BUFSIZE, in))) {
			if (ferror(in))
				return ErrFread;
			break;
		}

		if (fprintf(out, "%08lx: ", off) < 8)
			return ErrPrint;
		for (i = 0; i < n; ++i) {
			if (fprintf(out, "%02x", buf[i]) != 2)
				return ErrPrint;
			if (++i == n)
				break;
			fprintf(out, "%02x ", buf[i]);
			if (!((i+1) & (HEXCOLS-1))) {
				if (ULONG_MAX - off < HEXCOLS)
					return ErrFileBig;
				off += HEXCOLS;
				fputc(' ', out);
				if (printascii(out, buf+i - (HEXCOLS-1), HEXCOLS) != HEXCOLS)
					return ErrPrint;
				fputc('\n', out);
				if (i+1 != n)
					fprintf(out, "%08lx: ", off);
			}
		}

		rem = i & (HEXCOLS-1);
		if (!rem)
			continue;
		if (rem & 1) {
			if (fputs("   ", out) == EOF)
				return ErrPrint;
			for (j = 0; j < ((HEXCOLS-1) - rem)/2; ++j)
				fputs("     ", out);
		} else {
			if (fputs("     ", out) == EOF)
				return ErrPrint;
			for (j = 0; j < (HEXCOLS - rem)/2 - 1; ++j)
				fputs("     ", out);
		}
		fputc(' ', out);
		if (printascii(out, buf+i - rem, rem) != rem)
			return ErrPrint;
		fputc('\n', out);
	}

	return ErrNone;
}

static int incdump(FILE *out, FILE *in)
{
	unsigned long i, n, size = 0;

	if (fputs("unsigned char dump[] = {", out) == EOF)
		return ErrPrint;

	clearerr(in);
	for (;;) {
		if (!(n = fread(buf, 1, BUFSIZE, in))) {
			if (ferror(in))
				return ErrFread;
			break;
		}

		if (size > 0) {
			if (fprintf(out, ",0x%02x", buf[0]) != 5)
				return ErrPrint;
		} else {
			if (fprintf(out, "0x%02x", buf[0]) != 4)
				return ErrPrint;
		}

		for (i = 1; i < n; ++i)
			fprintf(out, ",0x%02x", buf[i]);

		if (ULONG_MAX - size < n)
			return ErrFileBig;
		size += n;
	}

	if (fprintf(out, "};\nunsigned long dumpsize = %lu;\n", size) < 31)
		return ErrPrint;

	return ErrNone;
}

/* TODO: implement revdump */
static int revdump(FILE *out, FILE *in)
{
	return ErrNoImpl;
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		return hexdump(stdout, stdin);
	} else if (argc == 2 && argv[1][0] == '-') {
		if (argv[1][1] == 'i' && argv[1][2] == '\0')
			return incdump(stdout, stdin);
		else if (argv[1][1] == 'r' && argv[1][2] == '\0')
			return revdump(stdout, stdin);
		else if (argv[1][1] == 'v' && argv[1][2] == '\0')
			return puts(VERSION), ErrNone;
	}

	fputs(USAGE, stderr);
	return ErrWrongArg;
}
