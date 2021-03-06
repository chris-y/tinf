/*
 * tgunzip - gzip decompressor example
 *
 * Copyright (c) 2003-2019 Joergen Ibsen
 * Copyright (c) 2020 Chris Young - ZX Next compatibility
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must
 *      not claim that you wrote the original software. If you use this
 *      software in a product, an acknowledgment in the product
 *      documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must
 *      not be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source
 *      distribution.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#include "tinf.h"

static unsigned char old_cpu_speed;

static unsigned long read_le32(const unsigned char *p)
{
	return ((unsigned long) p[0])
	     | ((unsigned long) p[1] << 8)
	     | ((unsigned long) p[2] << 16)
	     | ((unsigned long) p[3] << 24);
}

static void printf_error(const char *fmt, ...)
{
	va_list arg;

	fputs("tgunzip: ", stderr);

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);

	fputs("\n", stderr);
}

long main(int argc, char *argv[])
{
	unsigned char fin = 0;
	unsigned char fout = 0;
	unsigned char *source = NULL;
	unsigned char *dest = NULL;
	unsigned long dlen;
	uint32_t len, outlen;
	long retval = EXIT_FAILURE;
	long res;
	struct esx_stat es;

	printf("tgunzip " TINF_VER_STRING " - example from the tiny inflate library (www.ibsensoftware.com)\n\n");

	if (argc != 3) {
		fputs("usage: tgunzip INFILE OUTFILE\n\n"
		      "Both input and output are kept in memory, so do not use this on huge files.\n", stderr);
		return EXIT_FAILURE;
	}

	old_cpu_speed = ZXN_READ_REG(REG_TURBO_MODE);
	ZXN_NEXTREG(REG_TURBO_MODE, RTM_14MHZ);

	tinf_init();

	/* -- Open files -- */

	if ((fin = esx_f_open(argv[1], ESX_MODE_READ)) == 0) {
		printf_error("unable to open input file '%s'", argv[1]);
		goto out;
	}

	if ((fout = esx_f_open(argv[2], ESX_MODE_WRITE | ESX_MODE_OPEN_CREAT_NOEXIST)) == 0) {
		printf_error("unable to create output file '%s'", argv[2]);
		goto out;
	}

	/* -- Read source -- */

	if(esx_f_fstat(fin, (struct esx_stat *)&es)) {
		printf_error("unable to stat file");
		goto out;
	}

	len = es.size;

	if (len < 18) {
		printf_error("input too small to be gzip");
		goto out;
	}

	source = (unsigned char *) malloc(len);

	if (source == NULL) {
		printf_error("not enough memory allocating %ld bytes", len);
		goto out;
	}

	if (esx_f_read(fin, source, len) != len) {
		printf_error("error reading input file");
		goto out;
	}

	/* -- Get decompressed length -- */

	dlen = read_le32(&source[len - 4]);

	dest = (unsigned char *) malloc(dlen ? dlen : 1);

	if (dest == NULL) {
		printf_error("not enough memory");
		goto out;
	}

	/* -- Decompress data -- */

	outlen = dlen;

	res = tinf_gzip_uncompress(dest, &outlen, source, len);

	if ((res != TINF_OK) || (outlen != dlen)) {
		printf_error("decompression failed");
		goto out;
	}

	printf("decompressed %u bytes\n", outlen);

	/* -- Write output -- */

	esx_f_write(fout, dest, outlen);

	retval = EXIT_SUCCESS;

out:
	if (fin != NULL) {
		esx_f_close(fin);
	}

	if (fout != NULL) {
		esx_f_close(fout);
	}

	if (source != NULL) {
		free(source);
	}

	if (dest != NULL) {
		free(dest);
	}

	ZXN_NEXTREGA(REG_TURBO_MODE, old_cpu_speed);

	return retval;
}
