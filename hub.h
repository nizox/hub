#ifndef HUB_H
#define HUB_H

#include <event2/event.h>
#include "tun.h"

struct peer {
  char const *name;
  size_t received;
  size_t sent;
  struct sockaddr *addr;
  size_t addrlen;
  struct peer *next;
};

struct hub {
  size_t tx;
  size_t rx;
  short port;
  char *ipaddr;
  struct tnt_tun tun;
  struct event *ievent;
  struct event *levent;
  struct peer *peers;
};

void hub_init(struct hub *hub, struct event_base *evbase);
void hub_add_peer(struct hub *hub, char const *address);
void hub_start(struct hub *hub);
void hub_uninit(struct hub *hub);

#endif // !HUB_H
