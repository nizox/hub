#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <event2/event.h>
#include "hub.h"

#define DEFAULT_PORT 8989

static struct event_base *evbase;
static struct hub hub = {
  .port = DEFAULT_PORT,
  .ipaddr = NULL,
  .tun = {
    .fd = -1,
    .interface = NULL,
    .type = TNT_TAP
  }
};

static void
handle_quit(int sig)
{
  event_base_loopexit(evbase, NULL);
}

static void
usage(char const *progname)
{
  fprintf(stderr, "Usage: %s [-a addr/mask] [-i interface] [-p port] peer0 peer1 peerN\n",
      progname);
  exit(EXIT_FAILURE);
}

int
main(int argc, char * const argv[])
{
  int opt;
  struct sigaction sa;

  while ((opt = getopt(argc, argv, "a:i:p:")) != -1)
  {
    switch (opt)
    {
      case 'a':
        hub.ipaddr = strdup(optarg);
        break;
      case 'i':
        hub.tun.interface = strdup(optarg);
        break;
      case 'p':
        hub.port = (short) evutil_strtoll(optarg, NULL, 10);
        break;
      default:
        usage(argv[0]);
    }
  }

  for (; optind < argc; ++optind)
    hub_add_peer(&hub, argv[optind]);
  
  evbase = event_base_new();

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_quit;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  hub_init(&hub, evbase);
  hub_start(&hub);
  event_base_dispatch(evbase);
  hub_uninit(&hub);
  event_base_free(evbase);
  return EXIT_SUCCESS;
}
