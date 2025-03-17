#define VERSION "x 2025-03-17 https://github.com/danielsource/x.git"
#define USAGE "usage: x [-i|-r|-v] [< FILE]\n"

#include <stdio.h>

#ifdef _WIN32 /* for _setmode */
#include <io.h>
#include <fcntl.h>
#endif

#define HEXCOLS 16

#define BUFSIZE 8192
static unsigned char buf[BUFSIZE];

#if BUFSIZE % HEXCOLS != 0
#error "BUFSIZE must be multiple of HEXCOLS"
#endif

#if BUFSIZE < 80
#error "BUFSIZE must be large enough for line reading"
#endif

static const char bin2hex[] = "0123456789abcdef";

enum { ErrNone, ErrBadArg, ErrBadHex, ErrIO };

static int hexdump(FILE *out, FILE *in)
{
	unsigned long i, j, n, cols, nhex, off = 0;
	char hex[HEXCOLS*2 + HEXCOLS/2];

#ifdef _WIN32
	_setmode(_fileno(in), _O_BINARY);
#endif

	do {
		n = fread(buf, 1, BUFSIZE, in);
		if (ferror(in))
			return ErrIO;

		for (i = 0; i < n;) {
			fprintf(out, "%08lx: ", off);

			cols = n-i >= HEXCOLS ? HEXCOLS : n-i;

			for (j = nhex = 0; j < cols; j += 2) {
				hex[nhex++] = bin2hex[buf[i] >> 4];
				hex[nhex++] = bin2hex[buf[i] & 0xf];
				++i;
				if (i == n)
					break;
				hex[nhex++] = bin2hex[buf[i] >> 4];
				hex[nhex++] = bin2hex[buf[i] & 0xf];
				hex[nhex++] = ' ';
				++i;
			}

			for (; nhex < sizeof(hex); ++nhex)
				hex[nhex] = ' ';

			fwrite(hex, 1, sizeof(hex), out);

			fputc(' ', out);
			for (j = 0; j < cols; ++j)
				fputc(buf[i-cols+j] >= ' ' && buf[i-cols+j] <= '~'
				    ? buf[i-cols+j] : '.', out);
			fputc('\n', out);

			off += HEXCOLS;
		}
	} while (!feof(in));

	return ErrNone;
}

static int incdump(FILE *out, FILE *in)
{
	unsigned long i, n, sz = 0;

	fputs("unsigned char dump[] = {", out);

#ifdef _WIN32
	_setmode(_fileno(in), _O_BINARY);
#endif

	do {
		n = fread(buf, 1, BUFSIZE, in);
		if (ferror(in))
			return ErrIO;
		else if (!n)
			break;

		if (sz > 0)
			fputc(',', out);

		for (i = 0; i < n-1; ++i) {
			fputc('0', out);
			fputc('x', out);
			fputc(bin2hex[buf[i] >> 4], out);
			fputc(bin2hex[buf[i] & 0xf], out);
			fputc(',', out);
		}

		fputc('0', out);
		fputc('x', out);
		fputc(bin2hex[buf[i] >> 4], out);
		fputc(bin2hex[buf[i] & 0xf], out);

		sz += n;
	} while (!feof(in));

	fprintf(out, "};\nunsigned long dumpsize = %lu;\n", sz);

	return ErrNone;
}

/* XXX: revdump() simply ignores the offset info,
 * it does not have the same behavior of xxd (see fseek(3)). */
static int revdump(FILE *out, FILE *in)
{
	unsigned char *line, bytes[HEXCOLS];
	int nbyte;

#ifdef _WIN32
	_setmode(_fileno(out), _O_BINARY);
#endif

	for (;;) {
		line = buf;
		line[BUFSIZE-1] = '\01';
		if (!fgets((char *)line, BUFSIZE, in)) {
			return ferror(in) ? ErrIO : ErrNone;
		} else if (line[BUFSIZE-1] == '\0' && line[BUFSIZE-2] != '\n') {
			int c;

			while ((c = fgetc(in)) != EOF && c != '\n');
			return ErrBadHex;
		}

		while (*line++ != ':')
			if (*line == '\0')
				return ErrBadHex;

		for (nbyte = 0; nbyte < HEXCOLS; ++line) {
			if (line[0] == ' ') {
				if (line[1] == ' ')
					break;
				else
					continue;
			}

			switch (line[0]) {
			case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
				bytes[nbyte] = (line[0] - 'A' + 10) << 4;
				break;
			case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
				bytes[nbyte] = (line[0] - 'a' + 10) << 4;
				break;
			case '0':case '1':case '2':case '3':case '4':
			case '5':case '6':case '7':case '8':case '9':
				bytes[nbyte] = (line[0] - '0') << 4;
				break;
			default:
				return ErrBadHex;
			}

			switch (line[1]) {
			case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
				bytes[nbyte] += line[1] - 'A' + 10;
				break;
			case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
				bytes[nbyte] += line[1] - 'a' + 10;
				break;
			case '0':case '1':case '2':case '3':case '4':
			case '5':case '6':case '7':case '8':case '9':
				bytes[nbyte] += line[1] - '0';
				break;
			default:
				return ErrBadHex;
			}

			++nbyte;
			++line;
		}

		fwrite(bytes, 1, nbyte, out);
	}
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

	if (err == ErrBadArg)
		fputs(USAGE, stderr);
	else if (err == ErrIO)
		perror("x");

	return err;
}
