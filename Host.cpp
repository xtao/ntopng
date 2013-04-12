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
#include <netdb.h>

/* *************************************** */

Host::Host(NetworkInterface *_iface) : HashEntry(_iface) {
  ip = new IpAddress(0);
  initialize(false);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int32_t _ipv4) : HashEntry(_iface) {
  ip = new IpAddress(_ipv4);
  initialize(true);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, struct in6_addr _ipv6) : HashEntry(_iface) {
  ip = new IpAddress(_ipv6);
  initialize(true);
}

/* *************************************** */

Host::~Host() {
  if(symbolic_name) free(symbolic_name);
  delete ip;
}

/* *************************************** */

void Host::initialize(bool init_all) {
  num_uses = 0, name_resolved = false, symbolic_name = NULL;
  first_seen = last_seen = iface->getTimeLastPktRcvd();
  /* FIX - set ip.localHost */

  if(init_all) {
    char buf[64], rsp[256], str[64];
    snprintf(str, sizeof(str), "dns.cache.%s", ip->print(buf, sizeof(buf)));

    if(ntop->getPrefs()->get(str, rsp, sizeof(rsp)) == 0)
      symbolic_name = strdup(rsp);
    else {
      /* RPUSH dns.cache.toresolve <address> */
    }
  }
}

/* *************************************** */

void Host::dumpKeyToLua(lua_State* vm) {
  char buf[64];

  lua_pushstring(vm, get_name(buf, sizeof(buf)));
  lua_pushinteger(vm, sent.getNumBytes()+rcvd.getNumBytes());
  lua_settable(vm, -3);
}

/* ***************************************** */

bool Host::isIdle(u_int max_idleness) {
  if(num_uses > 0)
    return(false);
  else {
    HashEntry *h = (HashEntry*)this;

    return(h->isIdle(max_idleness));
  }
}

/* ***************************************** */

void Host::resolveHostName() {
  char hostname[NI_MAXHOST], sbuf[NI_MAXSERV];
  struct sockaddr sa;

  ip->dump(&sa);

  if(getnameinfo(&sa, sizeof(sa), hostname, NI_MAXHOST, sbuf, NI_MAXSERV, NI_NAMEREQD) == 0) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "-> %s", hostname);
    symbolic_name = strdup(hostname);
  }

  name_resolved = true;
}

/* ***************************************** */

void Host::setName(char *name) {
  if(symbolic_name) free(symbolic_name);
  symbolic_name = strdup(name);
}

/* ***************************************** */

char* Host::get_name(char *buf, u_int buf_len) {
  char addrbuf[64];

  if(symbolic_name != NULL)
    return(symbolic_name);

  snprintf(addrbuf, sizeof(addrbuf), "dns.cache.%s", get_ip()->print(buf, buf_len));
  if(ntop->getPrefs()->get(addrbuf, buf, buf_len) == 0) {
    setName(buf);
    return(symbolic_name);
  } else
    return(get_ip()->print(buf, buf_len));
}
