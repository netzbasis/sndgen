/*
 * Copyright (c) 2021 Benjamin Baier <ben@netzbasis.de>
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
#include <math.h>
#include <signal.h>
#include <sndio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "chirp.h"

/* CD format should always work, let's depend on it */
#define SG_SIG 1
#define SG_BITS 16
#define SG_PCHAN 2
#define SG_RATE 44100
#define SG_FRAMELEN SG_BITS/8*SG_PCHAN

int play = 1;

__dead static void usage(void);
static void handler(int);
static int fill_sine(int16_t*, int, int);

#define MIN(a,b) (((a)<(b))?(a):(b))

int
main(int argc, char *argv[])
{
	int ch;
	const char *errstr;
	int16_t buf[SG_RATE*SG_PCHAN] = {0};
	struct sio_hdl *hdl;
	struct sio_par par;
	size_t playlen = 0;
	int f_sine = 0;
	int f_lrmute = -1;
	int f_output = 0;
	int f_delay = 0;
	int ret = 1;
	int i;
	
	if (pledge("stdio dns unix rpath audio", NULL) == -1)
		err(1, "pledge");

	hdl = sio_open(SIO_DEVANY, SIO_PLAY, 0);
	if (hdl == NULL)
		errx(ret, "could not open device");

	if (pledge("stdio audio", NULL) == -1)
		err(1, "pledge");

	while ((ch = getopt(argc, argv, "cd:s:lor")) != -1) {
		switch (ch) {
		case 'c':
			playlen = MIN(chirp_pcm_len, sizeof(buf));
			memcpy(buf, chirp_pcm, playlen);
			break;
		case 'd':
			f_delay = strtonum(optarg, 1, 60, &errstr);
			if (errstr != NULL)
				errx(ret, "delay must be between 1 and 60");
			break;
		case 's':
			f_sine = strtonum(optarg, 20, 11000 , &errstr);
			if (errstr != NULL)
				errx(ret, "hz must be between 20 and 11000");
			playlen = fill_sine(buf, sizeof(buf), f_sine);
			break;
		case 'l':
			f_lrmute = 1;
			break;
		case 'o':
			f_output = 1;
			break;
		case 'r':
			f_lrmute = 0;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	if (playlen == 0) {
		playlen = sizeof(buf);
		arc4random_buf(buf, playlen);
	}

	if (f_lrmute != -1) {
		for (i=f_lrmute; i<SG_RATE*SG_PCHAN; i+=2)
			buf[i] = 0;
	}

	sio_initpar(&par);
	par.sig = SG_SIG;
	par.bits = SG_BITS;
	par.pchan = SG_PCHAN;
	par.rate = SG_RATE;
	par.le = SIO_LE_NATIVE;

	if (!sio_setpar(hdl, &par))
		goto cleanup;

	if (!sio_start(hdl))
		goto cleanup;

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (f_output) {
		/* undocumented debug dump to stdout */
		fwrite(buf, 1, playlen, stdout);
		ret=0;
		goto cleanup;
	}

	signal(SIGINT, handler);

	for (;;) {
		if (!play)
			break;
		if (!sio_write(hdl, buf, playlen))
			goto cleanup;
		for (i=f_delay; i > 0; i--) {
			if (!play)
				break;
			sleep(1);
		}
	}

	ret = 0;
cleanup:
	sio_stop(hdl);
	sio_close(hdl);
	if (ret)
		fprintf(stderr, "playback failed\n");
	return ret;
}

__dead static void
usage(void)
{
	fprintf(stderr, "usage: %s [-c] [-d delay] [-s hz] [-l | -r]\n", getprogname());
	exit(1);
}

static void
handler(int sig)
{
	play = 0;
}

static int
fill_sine(int16_t *buf, int bytelen, int hertz)
{
	int16_t amp;
	int pos;
	double rad = 0.0;
	int steps = SG_RATE/hertz;
	double stepwidth = (2*M_PI)/steps;
	
	for (pos=0; pos*SG_FRAMELEN < bytelen; pos++) {
		amp = (INT16_MAX-1)*sin(rad);
		*buf = amp;
		buf++;
		*buf = amp;
		buf++;
		rad += stepwidth;
	}
	/* last negative-to-positive transition */
	for (;pos % steps != 0; pos--);
	
	return pos*SG_FRAMELEN;
}
