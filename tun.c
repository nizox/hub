#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <event2/event.h>

#include "tun.h"

int
tnt_tun_close(struct tnt_tun *tun) {
  tapcfg_destroy(tun->handle);
  tun->handle = NULL;
  return 0;
}

int
tnt_tun_open(struct tnt_tun *tun) {
  if (tun->handle == NULL)
    tun->handle = tapcfg_init();

  if (tapcfg_start(tun->handle, tun->interface, 0) < 0)
    return (-1);

  tun->fd = tapcfg_get_fd(tun->handle);
  tun->interface = strdup(tapcfg_get_ifname(tun->handle));
  if (tun->interface == NULL)
    return (-1);
  return (0);
}

int
tnt_tun_iface_set_ipv4(struct tnt_tun *tun, char const *ipaddr) {
  int ret = -1;
  char *addr, *ptr;
  short netbits = 24;

  addr = strdup(ipaddr);
  if (addr == NULL)
    return (-1);
  ptr = strchr(addr, '/');
  if (ptr != NULL) {
    netbits = (short) evutil_strtoll(ptr + 1, NULL, 10);
    if (netbits < 1 || netbits > 32) {
      goto fail;
    }
    *ptr = '\0';
  }
  ret = tapcfg_iface_set_ipv4(tun->handle, addr, netbits);
fail:
  free(addr);
  return (ret);
}

int
tnt_tun_iface_set_status(struct tnt_tun *tun, int flags) {
  return tapcfg_iface_set_status(tun->handle, flags);
}
