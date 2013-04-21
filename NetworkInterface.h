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

#ifndef _NETWORK_INTERFACE_H_
#define _NETWORK_INTERFACE_H_

#include "ntop_includes.h"

#define NUM_ROOTS 512

class Flow;
class FlowHash;
class Host;
class HostHash;

typedef struct ether80211q {
  u_int16_t vlanId;
  u_int16_t protoType;
} Ether80211q;

class NetworkInterface {
 private:
  char *ifname;
  TrafficStats *ifStats;
  pcap_t *pcap_handle;
  int pcap_datalink_type;
  pthread_t pollLoop;

  FlowHash *flows_hash;
  /* Hosts */
  HostHash *hosts_hash;
  struct ndpi_detection_module_struct *ndpi_struct;
  time_t last_pkt_rcvd, next_idle_flow_purge, next_idle_host_purge;
  bool polling_started;

  void dropPrivileges();
  Flow* getFlow(u_int16_t vlan_id, const struct ndpi_iphdr *iph, u_int16_t ipsize, bool *src2dst_direction);

 public:
  NetworkInterface(char *name, bool change_user);
  ~NetworkInterface();
  void startPacketPolling();
  void shutdown();

  inline time_t getTimeLastPktRcvd()         { return(last_pkt_rcvd); };
  inline char* get_ndpi_proto_name(u_int id) { return(ndpi_get_proto_name(ndpi_struct, id)); };
  inline u_int get_flow_size()         { return(ndpi_detection_get_sizeof_ndpi_flow_struct()); };
  inline u_int get_size_id()           { return(ndpi_detection_get_sizeof_ndpi_id_struct());   };
  inline struct ndpi_detection_module_struct* get_ndpi_struct() { return(ndpi_struct);         };

  inline void incStats(time_t last, u_int pkt_len) { last_pkt_rcvd = last, ifStats->incStats(pkt_len); };
  inline TrafficStats* getStats()   { return(ifStats);                 };
  inline int get_datalink()           { return(pcap_datalink_type);      };
  inline pcap_t* get_pcap_handle()    { return(pcap_handle);             };

  void findFlowHosts(Flow *flow, Host **src, Host **dst);
  void packet_processing(const u_int64_t time, u_int16_t vlan_id,
			 const struct ndpi_iphdr *iph,
			 u_int16_t ipsize, u_int16_t rawsize);
  void dumpFlows();
  void getnDPIStats(NdpiStats *stats);
  void updateHostStats();
  void getActiveHostsList(lua_State* v, bool host_details);
  void getActiveFlowsList(lua_State* v);
  void getFlowPeersList(lua_State* vm);

  bool removeFlow(Flow *flow, bool lock_hash);
  void purgeIdleFlows();

  bool removeHost(Host *host, bool lock_hash);
  void purgeIdleHosts();

  u_int getNumFlows();
  u_int getNumHosts();

  void runHousekeepingTasks();
};

#endif /* _NETWORK_INTERFACE_H_ */
