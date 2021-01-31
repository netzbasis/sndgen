PROG=	sndgen

LDADD=	-lsndio
BINDIR=	/usr/local/bin

NOMAN=	noman
#MANDIR=	/usr/local/man/man

.include <bsd.prog.mk>
