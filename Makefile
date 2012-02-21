.include <bsd.own.mk>

PROG= hub

CFLAGS+= -Wall -I/usr/local/include/
LDFLAGS+= -L/usr/local/lib -L/usr/local/lib/event2
LDADD+=	-levent

.if defined(TARGET_OSNAME)
CFLAGS+= -D${TARGET_OSNAME}
.endif

SRCS= main.c hub.c

.if defined(TARGET_OSNAME) && ${TARGET_OSNAME} == "Linux"
SRCS+= tun-linux.c
.else
SRCS+= tun-bsd.c
.endif

.include <bsd.prog.mk>
