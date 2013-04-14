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
  char *data_dir, *callbacks_dir;
  NetworkInterface *iface;
  HTTPserver *httpd;
  NtopGlobals *globals;
  Redis *redis;
  PeriodicActivities *pa;
  Address *address;

 public:
  Ntop(Redis *_redis, char *_data_dir, char *_callbacks_dir);
  ~Ntop();

  void start();
  inline char* get_data_dir()                        { return(data_dir); }
  inline char* get_callbacks_dir()                   { return(callbacks_dir); }
  inline void registerInterface(NetworkInterface *i) { iface = i;        };
  inline void registerHTTPserver(HTTPserver *h)      { httpd = h;        };

  inline NetworkInterface* get_NetworkInterface(const char *name) { return(iface); }; /* FIX: check name */
  inline HTTPserver*       get_HTTPserver()       { return(httpd); };

  inline NtopGlobals*      getGlobals()           { return(globals); };
  inline Trace*            getTrace()             { return(globals->getTrace()); };
  inline Redis*            getRedis()             { return(redis);               };
};

extern Ntop *ntop;

#endif /* _NTOP_CLASS_H_ */
