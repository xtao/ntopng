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

  if (ntop->getPrefs()->save() < 0)
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Error saving preferences");

  delete ntop;
  exit(0);
}

/* ******************************************* */

int main(int argc, char *argv[]) {
  NetworkInterface *iface = NULL;
  HTTPserver *httpd = NULL;
  Redis *redis = NULL;
  Prefs *prefs = NULL;
  char *ifName;
  int rc;

  if((ntop = new Ntop()) == NULL) exit(0);
  if((prefs = new Prefs(ntop)) == NULL) exit(0);

  if((argc == 2) && (argv[1][0] != '-'))
    rc = prefs->loadFromFile(argv[1]);
  else
    rc = prefs->loadFromCLI(argc, argv);
  if(rc < 0) return(-1);

  if(prefs->get_redis_host() != NULL) redis = new Redis(prefs->get_redis_host(), prefs->get_redis_port());
  if(redis == NULL) redis = new Redis();
  
  ntop->registerPrefs(prefs, redis);

  prefs->loadUsersFromFile();

  ifName = ntop->get_if_name();
  if(ifName && ((strncmp(ifName, "tcp://", 6) == 0 || strncmp(ifName, "ipc://", 6) == 0))) {
    iface = new CollectorInterface("zmq-collector", ifName /* endpoint */, prefs->do_change_user());
  } else {
#ifdef HAVE_PF_RING
    try {
      iface = new PF_RINGInterface(ifName, prefs->do_change_user());
    } catch (int) {
#endif
      iface = new PcapInterface(ifName, prefs->do_change_user());
#ifdef HAVE_PF_RING
    }
#endif
  }

  if (prefs->get_cpu_affinity() >= 0)
    iface->set_cpu_affinity(prefs->get_cpu_affinity());

  ntop->registerInterface(iface);
  ntop->loadGeolocation(prefs->get_docs_dir());
  ntop->registerHTTPserver(httpd = new HTTPserver(prefs->get_http_port(), prefs->get_docs_dir(), prefs->get_scripts_dir()));

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

  if(prefs->get_categorization_key() != NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Host categorization is not enabled: using default key");
    ntop->setCategorization(new Categorization(prefs->get_categorization_key()));
    prefs->enable_categorization();
  }

  signal(SIGINT, sigproc);
  signal(SIGTERM, sigproc);
  signal(SIGINT, sigproc);

  ntop->start();
  iface->startPacketPolling();

  while(iface->isRunning()) {
    sleep(2);
    /* TODO - Do all this for all registered interfaces */
    iface->runHousekeepingTasks();
  }

  sigproc(0);
  delete ntop;

  return(0);
}

