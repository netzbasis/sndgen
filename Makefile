PROG=	sndgen

LDADD=	-lsndio -lm

BINDIR=	/usr/local/bin
MANDIR=	/usr/local/man/man

.include <bsd.prog.mk>
