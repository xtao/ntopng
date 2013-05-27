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
  getTrace()->traceEvent(TRACE_NORMAL, "Welcome to ntopng %s v.%s (%s) - (C) 1998-13 ntop.org",
			 PACKAGE_MACHINE, PACKAGE_VERSION, PACKAGE_RELEASE);  
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

  delete address;
  delete pa;
  delete geo;
  delete redis;
  delete globals;
}

/* ******************************************* */

void Ntop::start() {
  pa->startPeriodicActivitiesLoop();
  address->startResolveAddressLoop();
}

/* ******************************************* */

void Ntop::loadGeolocation(char *dir) {
  geo = new Geolocation(dir);
}
