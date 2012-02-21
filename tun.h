#ifndef TUN_H
#define TUN_H

/*
 */
struct tnt_tun {
  int fd;
  char *interface;
  enum {
    TNT_TUN,
    TNT_TAP
  } type;
};

int tnt_tun_open(struct tnt_tun *tun);
int tnt_tun_close(struct tnt_tun *tun);

#endif // !TUN_H
