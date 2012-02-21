#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_tun.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>

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
  short i = 0;
  int mode = IFF_BROADCAST;
  char path[PATH_MAX];

  if (tun->interface) {

    (void)snprintf(path, sizeof path, "/dev/%s", tun->interface);
    tun->fd = open(path, O_RDWR);
  } else {

    /* Open the tun interface by testing all devices */
    for (; i < 256; ++i) { /* XXX: magic value */

      (void)snprintf(path, sizeof path,
          (tun->type == TNT_TUN) ? "/dev/tun%hi" : "/dev/tap%hi", i);
      if ((tun->fd = open(path, O_RDWR)) > 0)
        break;
    }
  }

  if (tun->fd == -1 || i == 256) {
    warn("open failed: %s", path);
    return -1;
  }

  if (tun->type == TNT_TUN && ioctl(tun->fd, TUNSIFMODE, &mode) == -1) {
    warn("ioctl");
  }

  if (tun->interface == NULL) {
    tun->interface = strdup(path + 5); //path without "/dev/"
  }

  return tun->fd;
}
