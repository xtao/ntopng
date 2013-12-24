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
  bool localHost;
  u_int32_t host_serial;
  NdpiStats *ndpiStats;
  TrafficStats sent, rcvd;
  ActivityStats activityStats;
  HostContacts *contacts;
  /* Throughput */
  float bytes_thpt;
  ValueTrend bytes_thpt_trend;
  u_int64_t last_bytes;
  struct timeval last_update_time;
  time_t last_activity_update;
  
  void dumpStats(bool forceDump);
  void readStats();

 public:
  GenericHost(NetworkInterface *_iface);
  ~GenericHost();

  void dumpHostContacts(u_int16_t family_id);
  inline bool isLocalHost()          { return(localHost); };
  inline NdpiStats* get_ndpi_stats() { return(ndpiStats); };
  void incStats(u_int8_t l4_proto, u_int ndpi_proto, u_int64_t sent_packets, 
		u_int64_t sent_bytes, u_int64_t rcvd_packets, u_int64_t rcvd_bytes);
  inline void incrContact(NetworkInterface *iface, u_int32_t me_serial, IpAddress *peer, 
			  bool contacted_peer_as_client,
			  u_int family_id = HOST_FAMILY_ID, bool aggregated_host = false) {
    contacts->incrContact(iface, me_serial, peer, contacted_peer_as_client, 1,
			 family_id, aggregated_host); 
  }

  inline void flushContacts()         { contacts->purgeAll();                    };
  void getHostContacts(lua_State* vm) { contacts->getContacts(vm);               };
  bool hasHostContacts(char *host)    { return(contacts->hasHostContacts(host)); };
  inline u_int get_num_contacts_by(IpAddress* host_ip) { return(contacts->get_num_contacts_by(host_ip)); };
  inline u_int32_t get_host_serial()  { return(host_serial);               };
  void updateStats(struct timeval *tv);
  void updateActivities();
  inline ValueTrend getThptTrend()    { return(bytes_thpt_trend);          };
  inline char* getJsonActivityMap()   { return(activityStats.serialize()); };
  virtual char* get_string_key(char *buf, u_int buf_len) { return(NULL);   };
};

#endif /* _GENERIC_HOST_H_ */
