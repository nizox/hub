#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <event2/event.h>
#include "hub.h"

#define DEFAULT_INTERFACE "tap1"
#define DEFAULT_PORT 8989

static struct hub hub = {
  .interface = DEFAULT_INTERFACE,
  .port = DEFAULT_PORT,
};

static void
usage(char const *progname)
{
  fprintf(stderr, "Usage: %s [-i interface] [-p port] peer0 peer1 peerN\n",
      progname);
  exit(EXIT_FAILURE);
}

int
main(int argc, char * const argv[])
{
  int opt;
  struct event_base *evbase;

  while ((opt = getopt(argc, argv, "i:p:")) != -1)
  {
    switch (opt)
    {
      case 'i':
        hub.interface = optarg;
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
  hub_init(&hub, evbase);
  hub_start(&hub);
  event_base_dispatch(evbase);
  hub_uninit(&hub);
  event_base_free(evbase);
  return EXIT_SUCCESS;
}
