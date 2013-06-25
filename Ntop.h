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

#ifndef _NTOP_CLASS_H_
#define _NTOP_CLASS_H_

#include "ntop_includes.h"

class Ntop {
 private:
  char working_dir[MAX_PATH], install_dir[MAX_PATH], startup_dir[MAX_PATH];
  char *custom_ndpi_protos;
  NetworkInterface *iface;
  HTTPserver *httpd;
  NtopGlobals *globals;
  Redis *redis;
  PeriodicActivities *pa;
  AddressResolution *address;
  Prefs *prefs;
  Geolocation *geo;
  Categorization *categorization;
  Mutex *rrd_lock;

 public:
  Ntop(char *appName);
  void registerPrefs(Prefs *_prefs, Redis *_redis);
  ~Ntop();

  void setCustomnDPIProtos(char *path);
  inline char* getCustomnDPIProtos()                 { return(custom_ndpi_protos);                 };
  
  char* getValidPath(char *path);
  void loadGeolocation(char *dir);
  inline void setLocalNetworks(char *nets)           { address->setLocalNetworks(nets);            };
  inline bool isLocalAddress(int family, void *addr) { return(address->findAddress(family, addr)); };
  void start();
  inline Geolocation* getGeolocation()               { return(geo);            }
  inline char* get_if_name()                         { return(prefs->get_if_name()); }
  inline char* get_data_dir()                        { return(prefs->get_data_dir()); };
  inline char* get_callbacks_dir()                   { return(prefs->get_callbacks_dir()); };
  inline Categorization* get_categorization()        { return(categorization); };
 inline void registerInterface(NetworkInterface *i)  { iface = i;              };
  inline void registerHTTPserver(HTTPserver *h)      { httpd = h;              };
  inline void setCategorization(Categorization *c)   { categorization = c; };
  inline NetworkInterface* get_NetworkInterface(const char *name) { return(iface); }; /* FIX: check name */
  inline HTTPserver*       get_HTTPserver()          { return(httpd);            };
  inline char* get_working_dir()                     { return(working_dir);      };
  inline char* get_install_dir()                     { return(install_dir);      };
 
  inline NtopGlobals*      getGlobals()              { return(globals); };
  inline Trace*            getTrace()                { return(globals->getTrace()); };
  inline Redis*            getRedis()                { return(redis);               };
  inline Prefs*            getPrefs()                { return(prefs);               };

  inline void rrdLock(const char *filename, const int line)   { rrd_lock->lock(filename, line);   };
  inline void rrdUnlock(const char *filename, const int line) { rrd_lock->unlock(filename, line); };

  void getUsers(lua_State* vm);
  int  checkUserPassword(const char *user, const char *password);
  int  resetUserPassword(char *username, char *old_password, char *new_password);
  int  addUser(char *username, char *full_name, char *password);
  int  deleteUser(char *username);
  void setWorkingDir(char *dir);
  void fixPath(char *str);
  void removeTrailingSlash(char *str);
  void daemonize();
};

extern Ntop *ntop;

#endif /* _NTOP_CLASS_H_ */
