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
  delete m;
}

/* *************************************** */

void Host::initialize(bool init_all) {
  num_uses = 0, name_resolved = false, symbolic_name = NULL;
  first_seen = last_seen = iface->getTimeLastPktRcvd();
  /* FIX - set ip.localHost */

  m = new Mutex();

  if(init_all) {
    char buf[64], rsp[256], *host = ip->print(buf, sizeof(buf));

    if(ntop->getRedis()->getAddress(host, rsp, sizeof(rsp)) == 0)
      symbolic_name = strdup(rsp);
    else {
      ntop->getRedis()->queueHostToResolve(host);
    }
  }
}

/* *************************************** */

void Host::dumpKeyToLua(lua_State* vm, bool host_details) {
  char buf[64];

  if(host_details) {
    lua_newtable(vm);
    lua_push_str_table_entry(vm, "name", get_name(buf, sizeof(buf)));
    lua_push_int_table_entry(vm, "bytes.sent", sent.getNumBytes());
    lua_push_int_table_entry(vm, "bytes.rcvd", rcvd.getNumBytes());
    lua_pushstring(vm, get_ip()->print(buf, sizeof(buf)));
    lua_insert(vm, -2);
    lua_settable(vm, -3);
  } else {
    lua_pushstring(vm, get_name(buf, sizeof(buf)));
    lua_pushinteger(vm, sent.getNumBytes()+rcvd.getNumBytes());
    lua_settable(vm, -3);
  }
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

/*
  As this method can be called from Lua, in order to avoid concurency issues
  we need to lock/unlock
*/
void Host::setName(char *name) {
  m->lock(__FILE__, __LINE__);
  if(symbolic_name) free(symbolic_name);
  symbolic_name = strdup(name);
  m->unlock(__FILE__, __LINE__);
}

/* ***************************************** */

char* Host::get_name(char *buf, u_int buf_len) {
  char *addr;

  if(symbolic_name != NULL)
    return(symbolic_name);

  addr = get_ip()->print(buf, buf_len);
  if(ntop->getRedis()->getAddress(addr, buf, buf_len) == 0) {
    setName(buf);
    return(symbolic_name);
  } else
    return(addr);
}
