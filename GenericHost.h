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

#ifndef _GENERIC_HOST_H_
#define _GENERIC_HOST_H_

#include "ntop_includes.h"

typedef struct {
  IpAddress *host;
  u_int32_t num_contacts;
} HostContacts;

class GenericHost : public GenericHashEntry {
 protected:
  NdpiStats *ndpiStats;
  TrafficStats sent, rcvd;
  HostContacts clientContacts[MAX_NUM_HOST_CONTACTS], serverContacts[MAX_NUM_HOST_CONTACTS];  
  void incrHostContacts(IpAddress *peer, HostContacts *contacts);

 public:
  GenericHost(NetworkInterface *_iface);
  ~GenericHost();

  inline NdpiStats* get_ndpi_stats() { return(ndpiStats); };
  void incStats(u_int8_t l4_proto, u_int ndpi_proto, u_int64_t sent_packets, 
		u_int64_t sent_bytes, u_int64_t rcvd_packets, u_int64_t rcvd_bytes);
  void incrContact(char *peer, bool contacted_peer_as_client);
  inline void incrContact(IpAddress *peer, bool contacted_peer_as_client) { incrHostContacts(peer, contacted_peer_as_client ? clientContacts : serverContacts); }
  void getHostContacts(lua_State* vm);
};

#endif /* _GENERIC_HOST_H_ */
