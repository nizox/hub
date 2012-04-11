.include <bsd.own.mk>

.if !defined(TAPCFG_PATH)
.error "TAPCFG_PATH environment variable is missing"
.endif

PROG= hub

CFLAGS+= -Wall -I/usr/local/include/ -I${TAPCFG_PATH}/build/include
LDFLAGS+= -L/usr/local/lib -L/usr/local/lib/event2
LDADD+=	${TAPCFG_PATH}/build/libtapcfg.a -levent

.if defined(TARGET_OSNAME)
CFLAGS+= -D${TARGET_OSNAME}
.endif

SRCS= main.c hub.c tun.c

.include <bsd.prog.mk>
