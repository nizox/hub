.include <bsd.own.mk>

PROG= hub

CFLAGS+= -I/usr/local/include/
LDFLAGS+= -L/usr/local/lib/event2
LDADD+=	-levent

CFLAGS+= -D${TARGET_OSNAME}

SRCS= main.c hub.c

.include <bsd.prog.mk>
