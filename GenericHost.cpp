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

GenericHost::GenericHost(NetworkInterface *_iface) : GenericHashEntry(_iface) {
  ndpiStats = new NdpiStats();  
}

/* *************************************** */

GenericHost::~GenericHost() {
  char buf[64], *keyname;

  keyname = get_string_key(buf, sizeof(buf));
  if(keyname[0] != '\0') {
    char key[64];
    
    snprintf(key, sizeof(key), "%s.client", keyname);
    ntop->getRedis()->del(key);
    
    snprintf(key, sizeof(key), "%s.server", keyname);
    ntop->getRedis()->del(key);
  }

  if(ndpiStats)
    delete ndpiStats;
}

/* *************************************** */

void GenericHost::incStats(u_int8_t l4_proto, u_int ndpi_proto, 
			   u_int64_t sent_packets, u_int64_t sent_bytes,
			   u_int64_t rcvd_packets, u_int64_t rcvd_bytes) { 
  if(sent_packets || rcvd_packets) {
    sent.incStats(sent_packets, sent_bytes), rcvd.incStats(rcvd_packets, rcvd_bytes);

    if((ndpi_proto != NO_NDPI_PROTOCOL) && ndpiStats)
      ndpiStats->incStats(ndpi_proto, sent_packets, sent_bytes, rcvd_packets, rcvd_bytes);      
 
    updateSeen();
  }
}

/* *************************************** */

void GenericHost::incrContact(char *peer, bool contacted_peer_as_client) {
  char buf[64], *keyname;

  keyname = get_string_key(buf, sizeof(buf));
  if(keyname[0] != '\0') {
    char key[96];

    snprintf(key, sizeof(key), "%s.%s",
	     keyname, contacted_peer_as_client ? "client" : "server");
    
    ntop->getRedis()->zincrbyAndTrim(key, peer, 1 /* +1 */, MAX_NUM_HOST_CONTACTS);
  }
}

/* *************************************** */

void GenericHost::getHostContacts(lua_State* vm) {
  char key[64];

  get_string_key(key, sizeof(key));
  if(key[0] == '\0') return;

  lua_newtable(vm);

  /* client */
  ntop->getRedis()->getHostContacts(vm, this, true /* client */);
  lua_pushstring(vm, "client");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* server */
  ntop->getRedis()->getHostContacts(vm, this, false /* server */);
  lua_pushstring(vm, "server");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  lua_pushstring(vm, "contacts");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

