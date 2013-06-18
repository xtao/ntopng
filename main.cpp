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

extern "C" {
  extern char* rrd_strversion(void);
};

static const struct option long_options[] = {
  { "dns-mode",                          required_argument, NULL, 'n' },
  { "interface",                         required_argument, NULL, 'i' },
  { "data-dir",                          required_argument, NULL, 'd' },
  { "categorization-key",                required_argument, NULL, 'c' },
  { "http-port",                         required_argument, NULL, 'w' },
  { "local-networks",                    required_argument, NULL, 'm' },
  { "ndpi-protocols",                    required_argument, NULL, 'p' },
  { "redis",                             required_argument, NULL, 'r' },
  { "core-affinity",                     required_argument, NULL, 'g' },
  { "dont-change-user",                  no_argument,       NULL, 's' },
  { "disable-login",                     no_argument,       NULL, 'l' },
  { "verbose",                           no_argument,       NULL, 'v' },
  { "help",                              no_argument,       NULL, 'h' },
  /* End of options */
  { NULL,                                no_argument,       NULL,  0 }  
};

/* ******************************************* */

static void help() {
  printf("ntopng %s v.%s (%s) - (C) 1998-13 ntop.org\n\n"
	 "Usage: ntopng -m <local nets> [-d <data dir>] [-n mode] [-i <iface>]\n"
	 "              [-w <http port>] [-p <protos>] [-d <path>]\n"
	 "              [-c <categorization key>] [-r <redis>]\n"
	 "              [-l] [-s] [-v]\n\n"
	 "-n <mode>               | DNS address resolution mode\n"
	 "                        | 0 - Decode DNS responses and resolve local numeric IPs only (default)\n"
	 "                        | 1 - Decode DNS responses and resolve all numeric IPs\n"
	 "                        | 2 - Decode DNS responses and don't resolve numeric IPs\n"
	 "                        | 3 - Don't decode DNS responses and don't resolve numeric IPs\n"
	 "-i <interface>          | Input interface name\n"
	 "-d <path>               | Data directory (must be writable). Default: %s\n"
	 "-c <categorization key> | Key used to access host categorization services (default: disabled). \n"
         "                        | Please read README.categorization for more info.\n"
	 "-w <http port>          | HTTP port. Default: %u\n"
	 "-m <local network list> | List of local networks (e.g. -m \"192.168.0.0/24,172.16.0.0/16\")\n"
	 "-p <file>.protos        | Specify a nDPI protocol file (eg. protos.txt)\n"
	 "-r <redis host[:port]>  | Redis host[:port]\n"
	 "-g <cpu core id>        | Bind the capture/processing thread to a specific CPU Core\n"
	 "-s                      | Do not change user (debug only)\n"
	 "-l                      | Disable user login authentication\n"
	 "-v                      | Verbose tracing\n"
	 "-h                      | Help\n"
	 , PACKAGE_MACHINE, PACKAGE_VERSION, PACKAGE_RELEASE, CONST_DEFAULT_DATA_DIR, CONST_DEFAULT_NTOP_PORT);
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
    EthStats *stats = iface->getStats();

    stats->print();
    iface->shutdown();
  }

  delete ntop;
  exit(0);
}

/* ******************************************* */

int main(int argc, char *argv[]) {
  u_char c;
  char *ifName = NULL, *data_dir = strdup(CONST_DEFAULT_DATA_DIR), *docsdir =  (char*)"./httpdocs";
  u_int http_port = CONST_DEFAULT_NTOP_PORT;
  bool change_user = true, localnets = false;
  char *categorization_key = NULL;
  int cpu_affinity = -1;
  NetworkInterface *iface = NULL;
  HTTPserver *httpd = NULL;
  Redis *redis = NULL;
  Prefs *prefs = new Prefs();

  if((ntop = new Ntop()) == NULL) exit(0);

  while((c = getopt_long(argc, argv, "c:g:hi:w:r:sm:n:p:d:lv",
			 long_options, NULL)) != '?') {
    if(c == 255) break;

    switch(c) {
    case 'c':
      categorization_key = optarg;
      break;

    case 'g':
      cpu_affinity = atoi(optarg);
      break;

    case 'm':
      ntop->setLocalNetworks(optarg);
      localnets = true;
      break;

    case 'n':
      switch(atoi(optarg)) {
      case 0:
	break;
      case 1:
	prefs->resolve_all_hosts();
	break;
      case 2:
	prefs->disable_dns_resolution();
	break;
      case 3:
	prefs->disable_dns_resolution();
	prefs->disable_dns_responses_decoding();
	break;
      default:
	help();
      }
      break;

    case 'p':
      ntop->setCustomnDPIProtos(optarg);
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

    case 'd':
      free(data_dir);
      data_dir = strdup(optarg);
      break;

    case 'l':
      enable_users_login = false;
      break;

    case 'v':
      ntop->getTrace()->set_trace_level(MAX_TRACE_LEVEL);
      break;
    }
  }

  if(!localnets) help();

  if(redis == NULL) redis = new Redis();

  ntop->registerPrefs(prefs, redis, data_dir,
		      (char*)"./scripts/callbacks" /* Callbacks to call when specific events occour */);

  if(ifName && ((strncmp(ifName, "tcp://", 6) == 0 || strncmp(ifName, "ipc://", 6) == 0))) {
    iface = new CollectorInterface("zmq-collector", ifName /* endpoint */, change_user);
  } else {
#ifdef HAVE_PF_RING
    try {
      iface = new PF_RINGInterface(ifName, change_user);
    } catch (int) {
#endif
    iface = new PcapInterface(ifName, change_user);
#ifdef HAVE_PF_RING
    }
#endif
  }

  if (cpu_affinity >= 0)
    iface->set_cpu_affinity(cpu_affinity);

  ntop->registerInterface(iface);
  ntop->loadGeolocation(docsdir);
  ntop->registerHTTPserver(httpd = new HTTPserver(http_port, docsdir, "./scripts"));

  /*
    We have created the network interface and thus changed user. Let's not check
    if we can write on the data directory
  */
  {
    char path[256];
    FILE *fd;

    snprintf(path, sizeof(path), "%s/.test", ntop->get_data_dir());
    if((fd = fopen(path, "w")) == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR,
				   "Unable to write on %s: please specify a different directory (-d)",
				   ntop->get_data_dir());
      exit(0);
    } else
      fclose(fd); /* All right */
  }

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Using RRD version %s", rrd_strversion());

  if(categorization_key != NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Host categorization is not enabled: using default key");
    ntop->setCategorization(new Categorization(categorization_key));
    prefs->enable_categorization();
  }

  signal(SIGINT, sigproc);
  signal(SIGTERM, sigproc);
  signal(SIGINT, sigproc);

  ntop->start();
  iface->startPacketPolling();

  while(1) {
    sleep(2);
    /* TODO - Do all this for all registered interfaces */
    iface->runHousekeepingTasks();
  }

  sigproc(0);
  delete ntop;

  return(0);
}
