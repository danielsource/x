#define USAGE "usage: x [-i | -r]\n"

#include <limits.h>
#include <stdio.h>
#include <stdint.h>

#define HEXCOLS 16

#define BUFSIZE 8192
static unsigned char buf[BUFSIZE];

enum {
	ErrNone = 0,
	ErrWrongArg,
	ErrFread,
	ErrPrint,
	ErrFileBig,
	ErrCount
};

static int printascii(FILE *out, const unsigned char *data, size_t n)
{
	size_t i;
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
	unsigned long off = 0;
	size_t i, j, n, rem;

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
			fputs("   ", out);
			for (j = 0; j < ((HEXCOLS-1) - rem)/2; ++j)
				fputs("     ", out);
		} else {
			for (j = 0; j < (HEXCOLS - rem)/2; ++j)
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
	return ErrNone;
}

static int revdump(FILE *out, FILE *in)
{
	return ErrNone;
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
	}

	fputs(USAGE, stderr);
	return ErrWrongArg;
}
