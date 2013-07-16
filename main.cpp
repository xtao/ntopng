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

#if 0
  /* For the time being preferences are not saved. In the future this might change */
  
  if (ntop->getPrefs()->save() < 0)
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Error saving preferences");
#endif

#ifndef WIN32
  if (ntop->getPrefs()->get_pid_path() != NULL) 
    unlink(ntop->getPrefs()->get_pid_path());
#endif

  //delete ntop;
  exit(0);
}

/* ******************************************* */

#ifdef WIN32
extern "C" {
int ntop_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
 {
  NetworkInterface *iface = NULL;
  HTTPserver *httpd = NULL;
  Redis *redis = NULL;
  Prefs *prefs = NULL;
  char *ifName;
  int rc;

  if((ntop = new Ntop(argv[0])) == NULL) exit(0);
  if((prefs = new Prefs(ntop)) == NULL) exit(0);

  if((argc == 2) && (argv[1][0] != '-'))
    rc = prefs->loadFromFile(argv[1]);
  else
    rc = prefs->loadFromCLI(argc, argv);
  if(rc < 0) return(-1);

#ifndef WIN32
  if (prefs->get_pid_path() != NULL) {
    FILE *fd = fopen(prefs->get_pid_path(), "w");
    if(fd != NULL) {
      fprintf(fd, "%u\n", getpid());
      fclose(fd);
    } else ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to store PID in file %s", prefs->get_pid_path());
  }
#endif

  if(prefs->get_redis_host() != NULL) redis = new Redis(prefs->get_redis_host(), prefs->get_redis_port());
  if(redis == NULL) redis = new Redis();
  
  ntop->registerPrefs(prefs, redis);

  prefs->loadUsersFromFile();

  ifName = ntop->get_if_name();

  /* [ zmq-collector.lua@tcp://127.0.0.1:5556 ] */
  if(ifName && (strstr(ifName, "tcp://") || strstr(ifName, "ipc://"))) {
    char *at = strchr(ifName, '@');
    char *script, *endpoint;

    if(at != NULL) {
      u_int len = strlen(ifName)-strlen(at);

      script = (char*)malloc(len+1);
      if(script == NULL) {
	ntop->getTrace()->traceEvent(TRACE_ERROR, "Not enough memory");
	exit(-1);
      }

      strncpy(script, ifName, len);
      script[len] = '\0';
      endpoint = &at[1];
    } else
      script = strdup("nprobe-collector.lua"), endpoint = ifName;

    iface = new CollectorInterface(endpoint, script, prefs->do_change_user());
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

  if(prefs->daemonize_ntopng())
    ntop->daemonize();

  if (prefs->get_cpu_affinity() >= 0)
    iface->set_cpu_affinity(prefs->get_cpu_affinity());

  ntop->registerInterface(iface);
  ntop->loadGeolocation(prefs->get_docs_dir());
  ntop->registerHTTPserver(httpd = new HTTPserver(prefs->get_http_port(), 
						  prefs->get_docs_dir(), prefs->get_scripts_dir()));

  /*
    We have created the network interface and thus changed user. Let's not check
    if we can write on the data directory
  */
  {
    char path[MAX_PATH];
    FILE *fd;

    snprintf(path, sizeof(path), "%s/.test", ntop->get_working_dir());
    ntop->fixPath(path);

    if((fd = fopen(path, "w")) == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR,
				   "Unable to write on %s [%s]: please specify a different directory (-d)",
				   ntop->get_working_dir(), path);
      exit(0);
    } else {
      fclose(fd); /* All right */
      unlink(path);
    }
  }

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Using RRD version %s", rrd_strversion());

  if(prefs->get_categorization_key() != NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, 
				 "Host categorization is not enabled: using default key");
    ntop->setCategorization(new Categorization(prefs->get_categorization_key()));
    prefs->enable_categorization();
  }

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Working directory: %s", 
			       ntop->get_working_dir());
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Scripts/HTML pages directory: %s", 
			       ntop->get_install_dir());

  signal(SIGINT, sigproc);
  signal(SIGTERM, sigproc);
  signal(SIGINT, sigproc);

  #if defined(WIN32) && defined(DEMO_WIN32)
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "-----------------------------------------------------------");
    ntop->getTrace()->traceEvent(TRACE_WARNING, "This is a demo version of ntopng limited to %d packets", MAX_NUM_PACKETS);
	ntop->getTrace()->traceEvent(TRACE_WARNING, "Please go to http://shop.ntop.org for getting the full version");
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "-----------------------------------------------------------");
  #endif

  ntop->start();
  iface->startPacketPolling();

  while(iface->isRunning()) {
    sleep(2);
    /* TODO - Do all this for all registered interfaces */
    iface->runHousekeepingTasks();
  }

  sigproc(0);
  //delete ntop;

  return(0);
}
 
#ifdef WIN32
}
#endif
