#ifndef TUN_H
#define TUN_H

#include <tapcfg.h>

enum tnt_status {
  TNT_STATUS_ALL_UP = TAPCFG_STATUS_ALL_UP,
  TNT_STATUS_ALL_DOWN = TAPCFG_STATUS_ALL_DOWN
};

/*
 */
struct tnt_tun {
  int fd;
  char *interface;
  enum {
    TNT_TUN,
    TNT_TAP
  } type;
  tapcfg_t *handle;
};

int tnt_tun_open(struct tnt_tun *tun);
int tnt_tun_close(struct tnt_tun *tun);
int tnt_tun_iface_set_ipv4(struct tnt_tun *tun, char const *addr);
int tnt_tun_iface_set_status(struct tnt_tun *tun, int flags);

#endif // !TUN_H
