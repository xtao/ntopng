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

#ifdef WIN32
#include <shlobj.h> /* SHGetFolderPath() */
#else
#include <ifaddrs.h>
#endif

Ntop *ntop;

/* ******************************************* */

Ntop::Ntop(char *appName) {
  globals = new NtopGlobals();
  pa = new PeriodicActivities();
  address = new AddressResolution();
  categorization = NULL;
  httpbl = NULL;
  custom_ndpi_protos = NULL;
  rrd_lock = new Mutex(); /* FIX: one day we need to use the reentrant RRD API */
  prefs = NULL, redis = NULL;
  num_defined_interfaces = 0;
  local_interface_addresses = New_Patricia(128);
  export_interface = NULL;
  historical_interface_id = -1;
  start_time = 0; /* It will be initialized by start() */
  memset(iface, 0, sizeof(iface));
  httpd = NULL, runtimeprefs = NULL, geo = NULL;
#ifdef WIN32
  if(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,
		     SHGFP_TYPE_CURRENT, working_dir) != S_OK) {
    strcpy(working_dir, "C:\\Windows\\Temp" /* "\\ntopng" */); // Fallback: it should never happen
  }

  // Get the full path and filename of this program
  if(GetModuleFileName(NULL, startup_dir, sizeof(startup_dir)) == 0) {
    startup_dir[0] = '\0';
  } else {
    for(int i=(int)strlen(startup_dir)-1; i>0; i--)
      if(startup_dir[i] == '\\') {
	startup_dir[i] = '\0';
	break;
      }
  }
  strcpy(install_dir, startup_dir);
#else
  struct stat statbuf;
  char tmp_path[MAX_PATH];

  snprintf(working_dir, sizeof(working_dir), "%s/ntopng", CONST_DEFAULT_WRITABLE_DIR);

  umask (0);
  mkdir(working_dir, 0777);

  if(getcwd(startup_dir, sizeof(startup_dir)) == NULL)
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Occurred while checking the current directory (errno=%d)", errno);

  snprintf(tmp_path, sizeof(tmp_path), "%s/scripts/lua/index.lua", CONST_DEFAULT_INSTALL_DIR);

  if(stat(tmp_path, &statbuf) == 0)
    strcpy(install_dir, CONST_DEFAULT_INSTALL_DIR);
  else {
    snprintf(tmp_path, sizeof(tmp_path), "%s/scripts/lua/index.lua", CONST_ALT_INSTALL_DIR);
    
    if(stat(tmp_path, &statbuf) == 0)
      strcpy(install_dir, CONST_ALT_INSTALL_DIR);
    else {
      if(getcwd(install_dir, sizeof(install_dir)) == NULL)
	strcpy(install_dir, startup_dir);
    }
  }
#endif

  // printf("--> %s [%s]\n", startup_dir, appName);

  initTimezone();
}

/* ******************************************* */

/*
  Setup timezone differences

  We call it all the time as daylight can change
  during the night and thus we need to have it "fresh"
*/

void Ntop::initTimezone() {
  time_t now = time(NULL);
  time_offset =(long)(mktime(localtime(&now)) - mktime(gmtime(&now)));
}

/* ******************************************* */

Ntop::~Ntop() {
  for(int i=0; i<num_defined_interfaces; i++) {
    iface[i]->shutdown();
    delete(iface[i]);
  }

  if(httpbl) delete httpbl;
  if(httpd)  delete httpd;
  if(custom_ndpi_protos) delete(custom_ndpi_protos);

  Destroy_Patricia(local_interface_addresses, NULL);
  delete rrd_lock;
  delete address;
  delete pa;
  if(geo)   delete geo;
  if(redis) delete redis;
  delete globals;
  delete prefs;
  delete runtimeprefs;
}

/* ******************************************* */

void Ntop::registerPrefs(Prefs *_prefs) {
  struct stat statbuf;

  prefs = _prefs;

  if(stat(prefs->get_data_dir(), &statbuf)
     || (!(statbuf.st_mode & S_IFDIR)) /* It's not a directory */
     || (!(statbuf.st_mode & S_IWRITE)) /* It's not writable    */) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Invalid directory %s specified",
				 prefs->get_data_dir());
    _exit(-1);
  }

  if(stat(prefs->get_callbacks_dir(), &statbuf)
     || (!(statbuf.st_mode & S_IFDIR)) /* It's not a directory */
     || (!(statbuf.st_mode & S_IWRITE)) /* It's not writable    */) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Invalid directory %s specified",
				 prefs->get_callbacks_dir());
    _exit(-1);
  }

  if(prefs->get_local_networks()) {
    setLocalNetworks(prefs->get_local_networks());
  } else {
    /* Add defaults */
    /* http://www.networksorcery.com/enp/protocol/ip/multicast.htm */
    char *local_nets, buf[512];

    snprintf(buf, sizeof(buf), "%s,%s", CONST_DEFAULT_PRIVATE_NETS,
	     CONST_DEFAULT_LOCAL_NETS);
    local_nets = strdup(buf);
    setLocalNetworks(local_nets);
    free(local_nets);
  }

  memset(iface, 0, sizeof(iface));

  redis = new Redis(prefs->get_redis_host(), prefs->get_redis_port());
}
/* ******************************************* */

void Ntop::createHistoricalInterface() {
  HistoricalInterface *iface = new HistoricalInterface("Historical");
  prefs->add_network_interface((char *)"Historical", NULL);
  ntop->registerInterface(iface);
  historical_interface_id = iface->get_id();
}

/* ******************************************* */

NetworkInterface* Ntop::getHistoricalInterface() {
	return (getInterface(get_if_name(historical_interface_id))); 
};

/* ******************************************* */

void Ntop::createExportInterface() {
  if(prefs->get_export_endpoint())
    export_interface = new ExportInterface(prefs->get_export_endpoint());
  else
    export_interface = NULL;
}

/* ******************************************* */

void Ntop::start() {
  char daybuf[64], buf[32];
  time_t when = time(NULL);

  getTrace()->traceEvent(TRACE_NORMAL,
			 "Welcome to ntopng %s v.%s (%s) - (C) 1998-14 ntop.org",
			 PACKAGE_MACHINE, PACKAGE_VERSION, NTOPNG_SVN_RELEASE);

  start_time = time(NULL);

  strftime(daybuf, sizeof(daybuf), CONST_DB_DAY_FORMAT, localtime(&when));
  snprintf(buf, sizeof(buf), "%s.hostkeys", daybuf);

  pa->startPeriodicActivitiesLoop();
  if(categorization) categorization->startCategorizeCategorizationLoop();
  if(httpbl) httpbl->startHTTPBLLoop();

  runtimeprefs = new RuntimePrefs();

  prefs->loadIdleDefaults();
  loadLocalInterfaceAddress();

  for(int i=0; i<num_defined_interfaces; i++)
    iface[i]->startPacketPolling();

  sleep(2);
  address->startResolveAddressLoop();

  while(!globals->isShutdown()) {
    sleep(2);
    runHousekeepingTasks();
    // break;
  }
}

/* ******************************************* */

bool Ntop::isLocalAddress(int family, void *addr, int16_t *network_id) {
  *network_id = address->findAddress(family, addr);
  return(((*network_id) == -1) ? false : true);
};

/* ******************************************* */

bool Ntop::isLocalInterfaceAddress(int family, void *addr) {
  return((ptree_match(local_interface_addresses, family, addr,
		      (family == AF_INET) ? 32 : 128) != NULL) ? true /* found */ : false /* not found */);
}

/* ******************************************* */

void Ntop::loadLocalInterfaceAddress() {
#ifndef WIN32
  struct ifaddrs *local_addresses, *ifa;
  /* buf must be big enough for an IPv6 address(e.g. 3ffe:2fa0:1010:ca22:020a:95ff:fe8a:1cf8) */
  char buf[128], buf2[128];

  if(getifaddrs(&local_addresses) != 0) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to read interface addresses");
    return;
  }

  for(ifa = local_addresses; ifa != NULL; ifa = ifa->ifa_next) {
    if((ifa->ifa_addr == NULL)
       || ((ifa->ifa_flags & IFF_UP) == 0))
      continue;

    if(ifa->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in* s4 =(struct sockaddr_in *)(ifa->ifa_addr);

      if(inet_ntop(ifa->ifa_addr->sa_family,(void *)&(s4->sin_addr), buf, sizeof(buf)) != NULL) {
	int l = strlen(buf);

	snprintf(&buf[l], sizeof(buf)-l, "%s", "/32");
	ntop->getTrace()->traceEvent(TRACE_INFO, "Adding %s as IPv4 interface address", buf);
	strcpy(buf2, buf);
	ptree_add_rule(local_interface_addresses, buf);
	address->addLocalNetwork(buf2);
      }
    } else if(ifa->ifa_addr->sa_family == AF_INET6) {
      struct sockaddr_in6 *s6 =(struct sockaddr_in6 *)(ifa->ifa_addr);

      if(inet_ntop(ifa->ifa_addr->sa_family,(void *)&(s6->sin6_addr), buf, sizeof(buf)) != NULL) {
	int l = strlen(buf);

	snprintf(&buf[l], sizeof(buf)-l, "%s", "/128");
	ntop->getTrace()->traceEvent(TRACE_INFO, "Adding %s as IPv6 interface address", buf);
	strcpy(buf2, buf);
	ptree_add_rule(local_interface_addresses, buf);
	address->addLocalNetwork(buf2);
      }
    }
  }

  freeifaddrs(local_addresses);
#endif
}

/* ******************************************* */

void Ntop::loadGeolocation(char *dir) {
  if(geo != NULL) delete geo;
  geo = new Geolocation(dir);
}

/* ******************************************* */

void Ntop::setWorkingDir(char *dir) {
  snprintf(working_dir, sizeof(working_dir), "%s", dir);
  removeTrailingSlash(working_dir);
};

/* ******************************************* */

void Ntop::removeTrailingSlash(char *str) {
  int len = (int)strlen(str)-1;

  if((len > 0)
     && ((str[len] == '/') || (str[len] == '\\')))
    str[len] = '\0';
}

/* ******************************************* */

void Ntop::setCustomnDPIProtos(char *path) {
  if(path != NULL) {
    if(custom_ndpi_protos != NULL) free(custom_ndpi_protos);
    custom_ndpi_protos = strdup(path);
  }
}

/* ******************************************* */

void Ntop::getUsers(lua_State* vm) {
  char **usernames;
  char *username, *holder;
  char key[64], val[64];
  int rc, i;

  lua_newtable(vm);

  if((rc = ntop->getRedis()->keys("ntopng.user.*.password", &usernames)) <= 0) {
    return;
  }

  for (i = 0; i < rc; i++) {
    if(usernames[i] == NULL) continue; /* safety check */
    if(strtok_r(usernames[i], ".", &holder) == NULL) continue;
    if(strtok_r(NULL, ".", &holder) == NULL) continue;
    if((username = strtok_r(NULL, ".", &holder)) == NULL) continue;

    lua_newtable(vm);

    snprintf(key, sizeof(key), CONST_STR_USER_FULL_NAME, username);
    if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0)
      lua_push_str_table_entry(vm, "full_name", val);
    else
      lua_push_str_table_entry(vm, "full_name", (char*) "unknown");

    snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);
    if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0)
      lua_push_str_table_entry(vm, "group", val);
    else
      lua_push_str_table_entry(vm, "group", (char*)"unknown");

    snprintf(key, sizeof(key), CONST_STR_USER_NETS, username);
    if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0)
      lua_push_str_table_entry(vm, CONST_ALLOWED_NETS, val);
    else
      lua_push_str_table_entry(vm, CONST_ALLOWED_NETS, (char*)"");

    lua_pushstring(vm, username);
    lua_insert(vm, -2);
    lua_settable(vm, -3);

    free(usernames[i]);
  }

  free(usernames);
}

/* ******************************************* */

void Ntop::getUserGroup(lua_State* vm) {
  char key[64], val[64];
  char username[33];
  struct mg_connection *conn;

  lua_getglobal(vm, CONST_HTTP_CONN);
  if((conn = (struct mg_connection*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null HTTP connection");
    lua_push_str_table_entry(vm, "group", (char*)"unknown");
    return;
  }

  mg_get_cookie(conn, CONST_USER, username, sizeof(username));
  lua_newtable(vm);

  snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);
  if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0)
    lua_push_str_table_entry(vm, "group", val);
  else
    lua_push_str_table_entry(vm, "group", (char*)"unknown");
}

/* ******************************************* */
void Ntop::getAllowedNetworks(lua_State* vm) {
  char key[64], val[64];
  char username[33];
  struct mg_connection *conn;

  lua_getglobal(vm, CONST_HTTP_CONN);
  if((conn = (struct mg_connection*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null HTTP connection");
    lua_push_str_table_entry(vm, CONST_ALLOWED_NETS, (char*)"");
    return;
  }

  mg_get_cookie(conn, CONST_USER, username, sizeof(username));
  lua_newtable(vm);

  snprintf(key, sizeof(key), CONST_STR_USER_NETS, username);
  if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0)
    lua_push_str_table_entry(vm, CONST_ALLOWED_NETS, val);
  else
    lua_push_str_table_entry(vm, CONST_ALLOWED_NETS, (char*)"");
}
/* ******************************************* */

// Return 1 if username/password is allowed, 0 otherwise.
int Ntop::checkUserPassword(const char *user, const char *password) {
  char key[64], val[64];

  if((user == NULL) || (user[0] == '\0'))
    return(false);

  snprintf(key, sizeof(key), CONST_STR_USER_PASSWORD, user);

  if(ntop->getRedis()->get(key, val, sizeof(val)) < 0) {
    return(false);
  } else {
    char password_hash[33];

    mg_md5(password_hash, password, NULL);
    return(strcmp(password_hash, val) == 0);
  }
}

/* ******************************************* */

int Ntop::resetUserPassword(char *username, char *old_password, char *new_password) {
  char key[64];
  char password_hash[33];

  if(!checkUserPassword(username, old_password))
    return(false);

  snprintf(key, sizeof(key), CONST_STR_USER_PASSWORD, username);

  mg_md5(password_hash, new_password, NULL);

  if(ntop->getRedis()->set(key, password_hash, 0) < 0)
    return(false);

  return(true);
}

/* ******************************************* */

int Ntop::changeUserRole(char *username, char *usertype) const {
  char key[64];
  if(usertype != NULL) {
    snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);

    if(ntop->getRedis()->set(key, usertype, 0) < 0)
      return(false);
  }  
  
  return(true);
}

/* ******************************************* */

int Ntop::changeAllowedNets(char *username, char *allowed_nets) const {
  char key[64];

  if(allowed_nets != NULL) {
    snprintf(key, sizeof(key), CONST_STR_USER_NETS, username);

    if(ntop->getRedis()->set(key, allowed_nets, 0) < 0)
      return(false);
  }

  return(true);
}

/* ******************************************* */

int Ntop::addUser(char *username, char *full_name, char *password, char *host_role, char *allowed_networks) {
  char key[64];
  char password_hash[33];

  // FIX add a seed
  mg_md5(password_hash, password, NULL);

  ntop->getTrace()->traceEvent(TRACE_WARNING, "group = %s", host_role);
  ntop->getTrace()->traceEvent(TRACE_WARNING, "allowed networks = %s", allowed_networks);

  snprintf(key, sizeof(key), CONST_STR_USER_FULL_NAME, username);
  ntop->getRedis()->set(key, full_name, 0);

  snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);
  ntop->getRedis()->set(key, (char*) host_role /* TODO: done Paola */, 0);

  snprintf(key, sizeof(key), CONST_STR_USER_PASSWORD, username);
  ntop->getRedis()->set(key, password_hash, 0);

  snprintf(key, sizeof(key), CONST_STR_USER_NETS, username);
  ntop->getRedis()->set(key, allowed_networks, 0);

  return (true);
// tODO inserire controllo return return(ntop->getRedis()->set(key, password_hash, 0) >= 0);
}

/* ******************************************* */

int Ntop::deleteUser(char *username) {
  char key[64];

  snprintf(key, sizeof(key), CONST_STR_USER_FULL_NAME, username);
  ntop->getRedis()->del(key);

  snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);
  ntop->getRedis()->del(key);

  snprintf(key, sizeof(key), CONST_STR_USER_PASSWORD, username);
  return(ntop->getRedis()->del(key) >= 0);
}

/* ******************************************* */

void Ntop::fixPath(char *str) {
  for(int i=0; str[i] != '\0'; i++) {
#ifdef WIN32
    if(str[i] == '/') str[i] = '\\';
#endif
    
    if((i > 0) && (str[i] == '.') && (str[i-1] == '.')) {
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "Invalid path detected %s", str);
      str[i-1] = '_', str[i] = '_'; /* Invalidate the path */
    }
  }
}

/* ******************************************* */

char* Ntop::getValidPath(char *__path) {
  char _path[MAX_PATH];
  struct stat buf;
#ifdef WIN32
  const char *install_dir = (const char *)get_install_dir();
#endif
  const char* dirs[] = {
    startup_dir,
#ifndef WIN32
    CONST_DEFAULT_INSTALL_DIR,
#else
    install_dir,
#endif
    CONST_ALT_INSTALL_DIR,
    NULL
  };

  if(strncmp(__path, "./", 2) == 0) {
    snprintf(_path, MAX_PATH, "%s/%s", startup_dir, &__path[2]);
    fixPath(_path);

    if(stat(_path, &buf) == 0) {
      free(__path);
      return(strdup(_path));
    }
  }

  if((__path[0] == '/') || (__path[0] == '\\')) {
    /* Absolute paths */

    if(stat(__path, &buf) == 0) {
      return(__path);
    }
  } else
    snprintf(_path, MAX_PATH, "%s", __path);

  /* relative paths */
  for(int i=0; dirs[i] != NULL; i++) {
    char path[MAX_PATH];

    snprintf(path, sizeof(path), "%s/%s", dirs[i], _path);
    fixPath(path);

    if(stat(path, &buf) == 0) {
      free(__path);
      return(strdup(path));
    }
  }

  free(__path);
  return(strdup(""));
}

/* ******************************************* */

void Ntop::daemonize() {
#ifndef WIN32
  int childpid;

  signal(SIGHUP, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  if((childpid = fork()) < 0)
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Occurred while daemonizing (errno=%d)",
				 errno);
  else {
    if(!childpid) { /* child */
      int rc;

      //ntop->getTrace()->traceEvent(TRACE_NORMAL, "Bye bye: I'm becoming a daemon...");

#if 1
      rc = chdir("/");
      if(rc != 0)
	ntop->getTrace()->traceEvent(TRACE_ERROR, "Error while moving to / directory");

      setsid();  /* detach from the terminal */

      fclose(stdin);
      fclose(stdout);
      /* fclose(stderr); */

      /*
       * clear any inherited file mode creation mask
       */
      umask(0);

      /*
       * Use line buffered stdout
       */
      /* setlinebuf (stdout); */
      setvbuf(stdout, (char *)NULL, _IOLBF, 0);
#endif
    } else { /* father */
      ntop->getTrace()->traceEvent(TRACE_NORMAL,
				   "Parent process is exiting (this is normal)");
      _exit(0);
    }
  }
#endif
}

/* ******************************************* */

void Ntop::setLocalNetworks(char *_nets) {
  char *nets;
  u_int len;

  if(_nets == NULL) return;

  len = (u_int)strlen(_nets);

  if((len > 2)
     && (_nets[0] == '"')
     && (_nets[len-1] == '"')) {
    nets = strdup(&_nets[1]);
    nets[len-2] = '\0';
  } else
    nets = strdup(_nets);

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Setting local networks to %s", nets);
  address->setLocalNetworks(nets);
  free(nets);
};

/* ******************************************* */

NetworkInterface* Ntop::getNetworkInterface(const char *name) {
  /* This method accepts both interface names or Ids */
  u_int if_id = atoi(name);
  char str[8];

  snprintf(str, sizeof(str), "%u", if_id);
  if(strcmp(name, str) == 0) {
    /* name is a number */

    for(int i=0; i<num_defined_interfaces; i++) {
      if(iface[i]->get_id() == if_id)
	return(iface[i]);
    }

    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to find inteface Id %d", if_id);

    return(NULL);
  }

  for(int i=0; i<num_defined_interfaces; i++) {
    if(strcmp(iface[i]->get_name(), name) == 0)
      return(iface[i]);
  }

  /* FIX: remove this for at some point, when endpoint is passed */
  for(int i=0; i<num_defined_interfaces; i++) {
    char *script = iface[i]->getScriptName();
    if(script != NULL && strcmp(script, name) == 0)
      return(iface[i]);
  }

  /* Not found */
  if(!strcmp(name, "any"))
    return(iface[0]); /* FIX: remove at some point */

  return(NULL);
};

/* ******************************************* */

NetworkInterface* Ntop::getInterfaceById(u_int8_t id){
  for(int i=0; i<num_defined_interfaces; i++) {
    if(iface[i]->get_id() == id) {
      return(iface[i]);
    }
  }

  return(NULL);
}

/* ******************************************* */

NetworkInterface* Ntop::getInterface(char *name) {
 /* This method accepts both interface names or Ids */
  u_int if_id;
  char str[8];
  
  if(name == NULL) return(NULL);

  if_id = atoi(name);

  snprintf(str, sizeof(str), "%u", if_id);
  if(strcmp(name, str) == 0) {
    /* name is a number */

    return(getInterfaceById(if_id));
  }

  for(int i=0; i<num_defined_interfaces; i++) {
    if(strcmp(iface[i]->get_name(), name) == 0) {
      return(iface[i]);
    }
  }

  return(NULL);
}

/* ******************************************* */

int Ntop::getInterfaceIdByName(char *name) {
   /* This method accepts both interface names or Ids */
  u_int if_id = atoi(name);
  char str[8];

  snprintf(str, sizeof(str), "%u", if_id);
  if(strcmp(name, str) == 0) {
    /* name is a number */
    NetworkInterface *iface = getInterfaceById(if_id);
    
    if(iface != NULL)
      return(iface->get_id());
    else
      return(-1);
  }

  for(int i=0; i<num_defined_interfaces; i++) {
    if(strcmp(iface[i]->get_name(), name) == 0) {
      return(iface[i]->get_id());
    }
  }

  return(-1);
}

/* ******************************************* */

void Ntop::registerInterface(NetworkInterface *_if) {
  for(int i=0; i<num_defined_interfaces; i++) {
    if(strcmp(iface[i]->get_name(), _if->get_name()) == 0) {
      ntop->getTrace()->traceEvent(TRACE_WARNING,
				   "Skipping duplicated interface %s", _if->get_name());
      delete _if;
      return;
    }
  }

  if(num_defined_interfaces < MAX_NUM_DEFINED_INTERFACES) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Registered interface %s [id: %d]",
				 _if->get_name(), num_defined_interfaces);
    iface[num_defined_interfaces++] = _if;
    return;
  }

  ntop->getTrace()->traceEvent(TRACE_ERROR, "Too many networks defined");
};

/* ******************************************* */

void Ntop::runHousekeepingTasks() {
  if(globals->isShutdown()) return;

  for(int i=0; i<num_defined_interfaces; i++)
    iface[i]->runHousekeepingTasks();
}

/* ******************************************* */

void Ntop::shutdown() {
  for(int i=0; i<num_defined_interfaces; i++) {
    EthStats *stats = iface[i]->getStats();

    stats->print();
    iface[i]->shutdown();
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Interface %s [running: %d]",
				 iface[i]->get_name(), iface[i]->isRunning());
  }
}
