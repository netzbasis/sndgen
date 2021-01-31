PROG=	sndgen

LDADD=	-lsndio -lm
BINDIR=	/usr/local/bin

NOMAN=	noman
#MANDIR=	/usr/local/man/man

.include <bsd.prog.mk>
