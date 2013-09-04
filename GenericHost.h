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

class GenericHost : public GenericHashEntry {
 protected:
  NdpiStats *ndpiStats;
  TrafficStats sent, rcvd;
  ActivityStats activityStats;
  HostContacts contacts;
  /* Throughput */
  float bytes_thpt;
  ValueTrend bytes_thpt_trend;
  u_int64_t last_bytes;
  struct timeval last_update_time;

 public:
  GenericHost(NetworkInterface *_iface);
  ~GenericHost();

  inline NdpiStats* get_ndpi_stats() { return(ndpiStats); };
  void incStats(u_int8_t l4_proto, u_int ndpi_proto, u_int64_t sent_packets, 
		u_int64_t sent_bytes, u_int64_t rcvd_packets, u_int64_t rcvd_bytes);
  inline void incrContact(IpAddress *peer, bool contacted_peer_as_client) { contacts.incrContact(peer, contacted_peer_as_client); }
  void getHostContacts(lua_State* vm) { contacts.getIPContacts(vm); };
  void updateStats(struct timeval *tv);
  inline ValueTrend getThptTrend() { return(bytes_thpt_trend); }
};

#endif /* _GENERIC_HOST_H_ */
