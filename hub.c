#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <event2/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if defined(Linux)
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#endif

#include "hub.h"


#define MBUFSIZE 1500
char mbuf[MBUFSIZE];


#if !defined(Linux)
static char *
device_path_from_name(char const *name)
{
#define INTERFACE_PATH "/dev/%s"
  char *path;
  size_t len;

  len = strlen(INTERFACE_PATH) + strlen(name);
  path = malloc(len);
  if (!path) return NULL;
  snprintf(path, len, INTERFACE_PATH, name);
  return path;
}
#endif

static void
listen_callback(evutil_socket_t fd, short ev, void *arg)
{
  struct hub *hub = arg;
  ssize_t n;

  if (ev & EV_READ)
  {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    if ((n = recvfrom(fd, mbuf, sizeof(mbuf), 0,
            (struct sockaddr *) &addr, &addrlen)) > 0)
    {
      struct peer *ptr = hub->peers;

      while (ptr)
      {
        if (!evutil_sockaddr_cmp((struct sockaddr *) &addr, ptr->addr, 0))
        {
          ptr->received += n;
          printf("%zi bytes from %s (total %zi bytes)\n",
              n, ptr->name, ptr->received);
          break;
        }
        ptr = ptr->next;
      }
      n = write(event_get_fd(hub->ievent), mbuf, n);
      if (n > 0)
        hub->rx += n;
    }
  }
}

static void
broadcast_to_peers(struct hub *hub, const char *buf, size_t buflen)
{
  ssize_t n;
  struct peer *ptr = hub->peers;

  while (ptr)
  {
    n = sendto(event_get_fd(hub->levent), buf, buflen, 0,
        ptr->addr, ptr->addrlen);
    if (n > 0)
      ptr->sent += n;
    else
      fprintf(stderr, "Failed sending %i bytes to %s\n",
          buflen, ptr->name);
    ptr = ptr->next;
  }
}

static void
interface_callback(evutil_socket_t fd, short ev, void *arg)
{
  struct hub *hub = arg;
  ssize_t n;

  if (ev & EV_READ)
  {
    if ((n = read(fd, mbuf, sizeof(mbuf))) > 0)
    {
      hub->tx += n;
      printf("broadcasting %hi bytes (total %hi bytes)\n", n, hub->tx);
      broadcast_to_peers(hub, mbuf, n);
    }
  }
}

void
hub_init(struct hub *hub, struct event_base *evbase)
{
  int ifd, lfd;
  struct sockaddr_in addr;

#if defined(Linux)
  struct ifreq ifr;

  ifd = open("/dev/net/tun", O_RDWR);
  if (ifd == -1) err(1, NULL);

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strncpy(ifr.ifr_name, hub->interface, IFNAMSIZ);
  if (ioctl(ifd, TUNSETIFF, &ifr) == -1) err(1, NULL);
#else
  char *device = device_path_from_name(hub->interface);

  ifd = open(device, O_RDWR);
  free(device);
  if (ifd == -1) err(1, NULL);
#endif

  if (evutil_make_socket_nonblocking(ifd) == -1) err(1, NULL);

  hub->ievent = event_new(evbase, ifd, EV_READ | EV_PERSIST,
      interface_callback, hub);

  lfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (lfd == -1) err(1, NULL);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(hub->port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (evutil_make_socket_nonblocking(lfd) == -1) err(1, NULL);
  if (bind(lfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) err(1, NULL);
  if (evutil_make_listen_socket_reuseable(lfd) == -1) err(1, NULL);

  hub->levent = event_new(evbase, lfd, EV_READ | EV_PERSIST,
      listen_callback, hub);
}

void
hub_uninit(struct hub *hub)
{
  struct peer *ptr = hub->peers;

  evutil_closesocket(event_get_fd(hub->levent));
  evutil_closesocket(event_get_fd(hub->ievent));
  event_free(hub->levent);
  event_free(hub->ievent);

  while (ptr)
  {
    struct peer *tmp = ptr;

    free(ptr->addr);
    free(ptr);
    ptr = tmp;
  }
}

void
hub_start(struct hub *hub)
{
  event_add(hub->ievent, NULL);
  event_add(hub->levent, NULL);
}

void
hub_add_peer(struct hub *hub, char const *address)
{
  int e;
  char *hostname, *ptr, port[6];
  struct evutil_addrinfo hints, *answer = NULL;
  struct peer *peer;

  if ((ptr = strchr(address, ':')))
  {
    hostname = calloc(ptr - address + 1, sizeof(*hostname));
    if (!hostname) err(1, NULL);
    strncpy(hostname, address, ptr - address);
    strncpy(port, ptr + 1, sizeof(port));
  }
  else
  {
    hostname = (char *) address;
    evutil_snprintf(port, sizeof(port), "%hi", hub->port);
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = EVUTIL_AI_ADDRCONFIG;

  if ((e = evutil_getaddrinfo(hostname, port, &hints, &answer)) < 0)
  {
    fprintf(stderr, "Error while resolving '%s': %s", hostname,
        evutil_gai_strerror(e));
    return;
  }

  if (!answer)
    return;

  peer = calloc(1, sizeof(*peer));
  if (!peer) err(1, NULL);
  peer->name = address;

  peer->addrlen = answer->ai_addrlen;
  peer->addr = malloc(peer->addrlen);
  if (!peer->addr) err(1, NULL);
  memcpy(peer->addr, answer->ai_addr, peer->addrlen);

  peer->next = hub->peers;
  hub->peers = peer;

  evutil_freeaddrinfo(answer);
}

