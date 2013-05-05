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
  ip = new IpAddress(), ndpiStats = new NdpiStats();
  initialize(NULL, false);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int8_t mac[6], IpAddress *_ip) : HashEntry(_iface) {
  ip = new IpAddress(_ip), ndpiStats = new NdpiStats();
  initialize(mac, true);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int8_t mac[6]) : HashEntry(_iface) {
  ip = NULL;
  initialize(mac, true);
}

/* *************************************** */

Host::~Host() {
  if(symbolic_name) free(symbolic_name);
  if(ndpiStats) delete ndpiStats;
  delete ip;
  delete m;
}

/* *************************************** */

void Host::initialize(u_int8_t mac[6], bool init_all) {
  if(mac) memcpy(mac_address, mac, 6); else memset(mac_address, 0, 6);

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

char* Host::get_mac(char *buf, u_int buf_len) {
  snprintf(buf, buf_len,
	   "%02X:%02X:%02X:%02X:%02X:%02X",
	   mac_address[0] & 0xFF, mac_address[1] & 0xFF,
	   mac_address[2] & 0xFF, mac_address[3] & 0xFF,
	   mac_address[4] & 0xFF, mac_address[5] & 0xFF);

  return(buf);
}

/* *************************************** */

void Host::lua(lua_State* vm, bool host_details, bool returnHost) {
  char buf[64];

  if(host_details) {
    lua_newtable(vm);
    lua_push_str_table_entry(vm, "ip", ip->print(buf, sizeof(buf)));
    lua_push_str_table_entry(vm, "mac", get_mac(buf, sizeof(buf)));
    lua_push_str_table_entry(vm, "name", get_name(buf, sizeof(buf)));
    lua_push_int_table_entry(vm, "bytes.sent", sent.getNumBytes());
    lua_push_int_table_entry(vm, "bytes.rcvd", rcvd.getNumBytes());
    lua_push_int_table_entry(vm, "seen.first", first_seen);
    lua_push_int_table_entry(vm, "seen.last", last_seen);
    lua_push_int_table_entry(vm, "duration", get_duration());

    ndpiStats->lua(iface, vm);

    if(returnHost) {
      ;
    } else {
      if(ip != NULL)
	lua_pushstring(vm, ip->print(buf, sizeof(buf)));
      lua_insert(vm, -2);
      lua_settable(vm, -3);
    }
  } else {
    lua_pushstring(vm,  get_name(buf, sizeof(buf)));
    lua_pushinteger(vm, sent.getNumBytes()+rcvd.getNumBytes());
    lua_push_int_table_entry(vm, "duration", get_duration());
    lua_settable(vm, -3);
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
  if(ip == NULL) {
    return(get_mac(buf, buf_len));
  } else {
    char *addr, redis_buf[64];
    
    if(symbolic_name != NULL)
      return(symbolic_name);
    
    addr = ip->print(buf, buf_len);
    if(ntop->getRedis()->getAddress(addr, redis_buf, sizeof(redis_buf)) == 0) {
      setName(redis_buf);
      return(symbolic_name);
    } else
      return(addr);
  }
}

/* *************************************** */

void Host::incStats(u_int ndpi_proto, u_int32_t sent_packets, u_int32_t sent_bytes, u_int32_t rcvd_packets, u_int32_t rcvd_bytes) { 
    if(sent_packets || rcvd_packets) {
      sent.incStats(sent_packets, sent_bytes), rcvd.incStats(rcvd_packets, rcvd_bytes);
      if((ndpi_proto != NO_NDPI_PROTOCOL) && ndpiStats)
	ndpiStats->incStats(ndpi_proto, sent_packets, sent_bytes, rcvd_packets, rcvd_bytes);      
      updateSeen();
    }
  }

/* *************************************** */

int Host::compare(Host *h) {
  if(ip)
    return(ip->compare(h->ip));
  else
    return(memcmp(mac_address, h->mac_address, 6));
}

