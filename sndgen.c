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

/* CD format should always work, let's depend on it */
#define SG_SIG 1
#define SG_BITS 16
#define SG_PCHAN 2
#define SG_RATE 44100
#define SG_BIPBY 8

int play = 1;

__dead void static usage(void);
void handler(int);
int fill_sine(int16_t*, int, int);

int
main(int argc, char *argv[]) {
	int ch;
	const char *errstr;
	int16_t buf[SG_RATE*SG_PCHAN]; /* XXX 1sec audio buffer */
	struct sio_hdl *hdl;
	struct sio_par par;
	ssize_t playlen;
	int f_sine = 0;
	int ret = 1;
	int i;
	
	if (pledge("stdio dns unix rpath audio", NULL) == -1)
		err(1, "pledge");

	/* default to white noise bursts */
	playlen = sizeof(buf);
	arc4random_buf(&buf, playlen);
	memset(&buf, '\0', playlen/2);

	while ((ch = getopt(argc, argv, "s:lr")) != -1) {
		switch(ch) {
		case 's':
			f_sine = strtonum(optarg, 20, 20000 , &errstr);
			if (errstr != NULL)
				errx(ret, "hz must be between 20 and 20000");
			playlen = fill_sine(&buf[0], playlen, f_sine);
			break;
		case 'l':
			arc4random_buf(&buf, playlen);
			for (i=1; i<SG_RATE*SG_PCHAN; i+=2)
				buf[i] = 0;
			break;
		case 'r':
			arc4random_buf(&buf, playlen);
			for (i=0; i<SG_RATE*SG_PCHAN; i+=2)
				buf[i] = 0;
			break;
		default:
			usage();
		}
	}

	hdl = sio_open(SIO_DEVANY, SIO_PLAY, 0);
	if (hdl == NULL)
		errx(ret, "could not open device");

	if (pledge("stdio audio", NULL) == -1)
		err(1, "pledge");

	sio_initpar(&par);
	par.sig = SG_SIG;
	par.bits = SG_BITS;
	par.pchan = SG_PCHAN;
	par.rate = SG_RATE;
	par.le = 1; /* XXX what about be architectures? */

	if (!sio_setpar(hdl, &par))
		goto cleanup;

	if (!sio_start(hdl))
		goto cleanup;

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	signal(SIGINT, handler);

	for (;;) {
		if (!sio_write(hdl, buf, playlen))
			goto cleanup;
		if (!play)
			break;
	}

	ret = 0;
cleanup:
	if (ret)
		fprintf(stderr, "playback failed\n");
	sio_close(hdl);
	return ret;
}

__dead void static
usage(void) {
	fprintf(stderr, "usage: %s [-s hz] [-l | -r]\n", getprogname());
	exit(1);
}

void
handler(int sig)
{
	play = 0;
}

int
fill_sine(int16_t *buf, int bytelen, int hertz)
{
	int16_t amp;
	int pos = 0;
	double rad = 0.0;
	int steps = SG_RATE/hertz;
	double stepwidth = (2*M_PI)/steps;
	
	while (pos*SG_BITS/SG_BIPBY*SG_PCHAN < bytelen) {
		amp = (INT16_MAX-1)*sin(rad);
		*buf = amp;
		buf++;
		*buf = amp;
		buf++;

		rad += stepwidth;
		pos++;
	}
	/* find last 0 transition */
	for (;pos % steps != 0;pos--);
	
	return pos*SG_BITS/SG_BIPBY*SG_PCHAN;
}
