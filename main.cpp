/*
 *
 * (C) 2013 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

/* ******************************************* */

static void help() {
  printf("ntopng %s v.%s (%s) - (C) 1998-13 ntop.org\n\n"
	 "-n <mode>              | DNS address resolution mode\n"
	 "                       | 0 - Decode DNS responses and resolve numeric IPs\n"
	 "                       | 1 - Decode DNS responses and don't resolve numeric IPs\n"
	 "                       | 2 - Don't decode DNS responses and don't resolve numeric IPs\n"
	 "-i <interface>         | Input interface name\n"
	 "-w <http port>         | HTTP port\n"
	 "-r <redis host[:port]> | Redis host[:port]\n"
	 "-s                     | Do not change user (debug only)\n"
	 , PACKAGE_MACHINE, PACKAGE_VERSION, PACKAGE_RELEASE);
  exit(0);
}

/* ******************************** */

void sigproc(int sig) {
  static int called = 0;

  if(called) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Ok I am leaving now");
    exit(0);
  } else {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Shutting down...");
    called = 1;
  }

  ntop->getGlobals()->shutdown();
  sleep(2); /* Wait until all threads know that we're shutting down... */  

  if(NetworkInterface *iface = ntop->get_NetworkInterface("any")) {
    TrafficStats *stats = iface->getStats();

    stats->printStats();
    iface->shutdown();
  }

  delete ntop;
  exit(0);
}

/* ******************************************* */

int main(int argc, char *argv[]) {
  u_char c;
  char *ifName = NULL;
  u_int http_port = 3000;
  bool change_user = true;
  NetworkInterface *iface = NULL;
  HTTPserver *httpd = NULL;
  Redis *redis = NULL;
  Prefs *prefs = new Prefs();

  while((c = getopt(argc, argv, "hi:w:r:sn:")) != '?') {
    if(c == 255) break;

    switch(c) {
    case 'n':
      switch(atoi(optarg)) {
      case 0:
	break;
      case 1:
	prefs->disable_dns_resolution();
	break;
      case 2:
	prefs->disable_dns_resolution();
	prefs->disable_dns_responses_decoding();
	break;
      default:
	help();
      }
      break;

    case 'h':
      help();
      break;

    case 'i':
      ifName = optarg;
      break;

    case 'w':
      http_port = atoi(optarg);
      break;

    case 'r':
      {
	char *host;
	int port;
	char buf[64];

	snprintf(buf, sizeof(buf), "%s", optarg);
	host = strtok(buf, ":");

	if(host) {
	  char *c = strtok(NULL, ":");

	  if(c)
	    port = atoi(c);
	  else
	    port = 6379;
	}

	redis = new Redis(host, port);
      }
      break;

    case 's':
      change_user = false;
      break;
    }
  }

  if((ntop = new Ntop()) == NULL) exit(0);

  if(redis == NULL) redis = new Redis();

  ntop->registerPrefs(prefs, redis, 
		      (char*)"./data" /* Directory where ntopng will dump data: make sure it can write it there */,
		      (char*)"./scripts/callbacks" /* Callbacks to call when specific events occour */);

  ntop->registerInterface(iface = new NetworkInterface(ifName, change_user));
  ntop->registerHTTPserver(httpd = new HTTPserver(http_port, "./httpdocs", "./scripts/lua"));

  signal(SIGINT, sigproc);
  signal(SIGTERM, sigproc);
  signal(SIGINT, sigproc);

  ntop->start();
  iface->startPacketPolling();

#if 1
  while(1) {
    NdpiStats stats;

    sleep(3);
    iface->updateHostStats();
    iface->getnDPIStats(&stats);
    //stats.print(iface);

    //iface->dumpFlows();
  }
#else
  sleep(3);
#endif

  sigproc(0);
  delete ntop;

  return(0);
}
