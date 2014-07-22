/*
 *
 * (C) 2013-14 - ntop.org
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
  ntop->shutdown();

#if 0
  /* For the time being preferences are not saved. In the future this might change */

  if(ntop->getPrefs()->save() < 0)
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Error saving preferences");
#endif

#ifndef WIN32
  if(ntop->getPrefs()->get_pid_path() != NULL) {
    int rc = unlink(ntop->getPrefs()->get_pid_path());
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Deleted PID %s [rc: %d]",
				 ntop->getPrefs()->get_pid_path(), rc);
  }
#endif

  delete ntop;
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
  HTTPserver *httpd = NULL;
  Prefs *prefs = NULL;
  char *ifName;
  int rc;

  if((ntop = new(std::nothrow)  Ntop(argv[0])) == NULL) exit(0);
  if((prefs = new(std::nothrow) Prefs(ntop)) == NULL)   exit(0);

  if((argc == 2) && (argv[1][0] != '-'))
    rc = prefs->loadFromFile(argv[1]);
  else
    rc = prefs->loadFromCLI(argc, argv);

  if(rc < 0) return(-1);

  ntop->registerPrefs(prefs);

  if(prefs->get_num_user_specified_interfaces() == 0) {
    /* We add all interfaces avilable on this host */
    prefs->add_default_interfaces();
  }

  if(prefs->daemonize_ntopng())
    ntop->daemonize();

  for(int i=0; i<max_val(1, prefs->get_num_user_specified_interfaces()); i++) {
    NetworkInterface *iface;

    ifName = ntop->get_if_name(i);

    /* [ zmq-collector.lua@tcp://127.0.0.1:5556 ] */
    if(ifName && (strstr(ifName, "tcp://")
		  || strstr(ifName, "ipc://"))
       ) {
      char *at = strchr(ifName, '@');
      char *topic = (char*)"flow", *endpoint;

      if(at != NULL)
	endpoint = &at[1];
      else
	endpoint = ifName;

      iface = new CollectorInterface(i, endpoint, topic);
    } else {
#ifdef HAVE_PF_RING
      try {
	iface = new PF_RINGInterface(i, ifName);
      } catch (int) {
#endif
	iface = new PcapInterface(i, ifName);
#ifdef HAVE_PF_RING
      }
#endif
    }

    if(prefs->get_cpu_affinity() >= 0)
      iface->set_cpu_affinity(prefs->get_cpu_affinity());

    if(prefs->get_packet_filter() != NULL)
      iface->set_packet_filter(prefs->get_packet_filter());

    ntop->registerInterface(iface);
  }

  // Create empty and not running historical interface
  if(prefs->do_dump_flows_on_db())
    ntop->createHistoricalInterface();

  ntop->createExportInterface();

  if(prefs->do_change_user())
    Utils::dropPrivileges();

#ifndef WIN32
  if(prefs->get_pid_path() != NULL) {
    FILE *fd;

    fd = fopen(prefs->get_pid_path(), "w");
    if(fd != NULL) {
      fprintf(fd, "%u\n", getpid());
      fclose(fd);
      chmod(prefs->get_pid_path(), 0777);
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "PID stored in file %s",
				   prefs->get_pid_path());
    } else
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to store PID in file %s",
				   prefs->get_pid_path());
  }
#endif

  ntop->loadGeolocation(prefs->get_docs_dir());
  ntop->registerHTTPserver(httpd = new HTTPserver(prefs->get_http_port(),
						  prefs->get_docs_dir(),
						  prefs->get_scripts_dir()));
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

  if(prefs->get_httpbl_key() != NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING,
        "HTTPBL categorization is not enabled: using default key");
    ntop->setHTTPBL(new HTTPBL(prefs->get_httpbl_key()));
    prefs->enable_httpbl();
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

  sigproc(0);
  delete ntop;

  return(0);
}

#ifdef WIN32
}
#endif
