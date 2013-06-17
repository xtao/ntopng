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

Ntop *ntop;

/* ******************************************* */

Ntop::Ntop() {
  globals = new NtopGlobals();
  pa = new PeriodicActivities();
  address = new Address();
  categorization = NULL;
  custom_ndpi_protos = NULL;
  rrd_lock = new Mutex(); /* FIX: one day we need to use the reentrant RRD API */
}

/* ******************************************* */

void Ntop::registerPrefs(Prefs *_prefs, Redis *_redis, char *_data_dir, char *_callbacks_dir) {
  struct stat statbuf;

  prefs = _prefs, redis = _redis;

  if(stat(_data_dir, &statbuf)
     || (!(statbuf.st_mode & S_IFDIR)) /* It's not a directory */
     || (!(statbuf.st_mode & S_IWRITE)) /* It's not writable    */) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Invalid directory %s specified", _data_dir);
    exit(-1);
  }

  if(stat(_callbacks_dir, &statbuf)
     || (!(statbuf.st_mode & S_IFDIR)) /* It's not a directory */
     || (!(statbuf.st_mode & S_IWRITE)) /* It's not writable    */) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Invalid directory %s specified", _callbacks_dir);
    exit(-1);
  }

  data_dir = strdup(_data_dir), callbacks_dir = strdup(_callbacks_dir);
}

/* ******************************************* */

Ntop::~Ntop() {
  if(iface) delete iface;
  if(httpd) delete httpd;
  if(custom_ndpi_protos) delete(custom_ndpi_protos);

  delete rrd_lock;
  delete address;
  delete pa;
  delete geo;
  delete redis;
  delete globals;
}

/* ******************************************* */

void Ntop::start() {
  getTrace()->traceEvent(TRACE_NORMAL, "Welcome to ntopng %s v.%s (%s) - (C) 1998-13 ntop.org",
			 PACKAGE_MACHINE, PACKAGE_VERSION, PACKAGE_RELEASE);  

  pa->startPeriodicActivitiesLoop();
  address->startResolveAddressLoop();
  if(categorization) categorization->startCategorizeCategorizationLoop();
}

/* ******************************************* */

void Ntop::loadGeolocation(char *dir) {
  geo = new Geolocation(dir);
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

  lua_newtable(vm);

  /* TODO */

  lua_newtable(vm);
  lua_push_str_table_entry(vm, "full_name", (char*) "unknown");
  lua_push_str_table_entry(vm, "group",     (char*) "administrator");
  lua_pushstring(vm, "admin");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

}

/* ******************************************* */

// Return 1 if username/password is allowed, 0 otherwise.
int Ntop::checkUserPassword(const char *user, const char *password) {
  // In production environment we should ask an authentication system
  // to authenticate the user.
  char key[64], val[64];
  char password_hash[33];
  
  if(user == NULL) return(false);

  snprintf(key, sizeof(key), "user.%s", user);

  if(ntop->getRedis()->get(key, val, sizeof(val)) < 0)
    return(false);
  else {
    // FIX add a seed when users management will be available on the web gui
    mg_md5(password_hash, password, NULL);
    return(strcmp(password_hash, val) == 0);
  }
}


/* ******************************************* */

int Ntop::resetUserPassword(char *username, char *old_password, char *new_password) {
  char key[64];
  char password_hash[33]; 

  if (!checkUserPassword(username, old_password))
    return(false);
  
  snprintf(key, sizeof(key), "user.%s", username);

  // FIX add a seed when users management will be available on the web gui
  mg_md5(password_hash, new_password, NULL);

  if(ntop->getRedis()->set(key, password_hash, sizeof(password_hash)) < 0) 
    return(false); 

  return(true);
}

/* ******************************************* */

