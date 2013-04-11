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

/* *************************************** */

Host::Host(NetworkInterface *_iface) : HashEntry(_iface) {
  ip = new IpAddress(0);
  initialize();
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int32_t _ipv4) : HashEntry(_iface) {
  ip = new IpAddress(_ipv4);
  initialize();
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, struct in6_addr _ipv6) : HashEntry(_iface) {
  ip = new IpAddress(_ipv6);
  initialize();
}

/* *************************************** */

Host::~Host() {
  delete ip;
}

/* *************************************** */

void Host::initialize() {
  num_uses = 0;
  first_seen = last_seen = iface->getTimeLastPktRcvd();
  /* FIX - set ip.localHost */
}

/* *************************************** */

void Host::dumpKeyToLua(lua_State* vm) {
  char str[64];

  lua_pushstring(vm, get_ip()->print(str, sizeof(str)));
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
