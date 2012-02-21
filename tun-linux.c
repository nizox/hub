#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "tun.h"

int
tnt_tun_close(struct tnt_tun *tun) {
  if (tun->fd == -1) {
    warnx("nothing to close");
  }
  return close(tun->fd);
}

int
tnt_tun_open(struct tnt_tun *tun) {
	struct ifreq ifr;

	if ((tun->fd = open("/dev/net/tun", O_RDWR)) == -1) {
		warn("open");
		return (-1);
	}

	memset(&ifr, 0, sizeof(ifr));
        if (tun->interface) {
          strncpy(ifr.ifr_name, tun->interface, IFNAMSIZ - 1);
        }
	ifr.ifr_flags = (tun->type == TNT_TUN) ? IFF_TUN : IFF_TAP;
	ifr.ifr_flags |= IFF_NO_PI;
	ifr.ifr_flags |= IFF_BROADCAST;

	if (ioctl(tun->fd, TUNSETIFF, &ifr) == -1) {
		warn("ioctl TUNSETIFF");
		goto failed;
	}

        if (tun->interface == NULL
            && ioctl(tun->fd, TUNGETIFF, &ifr) == -1) {
		warn("ioctl TUNGETIFF");
		goto failed;
        }

        tun->interface = strdup(ifr.ifr_name);
        if (tun->interface == NULL) {
          warn("malloc failed");
          goto failed;
        }

	return (tun->fd);

 failed:
	close(tun->fd);
        tun->fd = -1;
	return (-1);
}
