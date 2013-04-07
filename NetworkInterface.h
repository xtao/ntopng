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

typedef struct ether80211q {
  u_int16_t vlanId;
  u_int16_t protoType;
} Ether80211q;

class NetworkInterface {
 private:
  char *ifname;
  InterfaceStats *ifStats;
  pcap_t *pcap_handle;
  int pcap_datalink_type;
  struct ndpi_flow *ndpi_flows_root[NUM_ROOTS];
  u_int32_t ndpi_flow_count;
  pthread_t pollLoop;
  /* Hosts */
  u_int32_t num_hosts;
  Host *hosts_root;

  Flow* getFlow(u_int16_t vlan_id, const struct ndpi_iphdr *iph, u_int16_t ipsize);

 public:
  NetworkInterface(char *name);
  ~NetworkInterface();
  void startPacketPolling();
  void shutdown();
  inline void incStats(u_int pkt_len) { ifStats->incStats(pkt_len);      };  
  inline InterfaceStats* getStats()   { return(ifStats);                 };
  inline int get_datalink()           { return(pcap_datalink_type);      };
  inline pcap_t* get_pcap_handle()    { return(pcap_handle);             };

  void findFlowHosts(Flow *flow, Host **src, Host **dst);
  void packet_processing(const u_int64_t time, u_int16_t vlan_id,
			 const struct ndpi_iphdr *iph,
			 u_int16_t ipsize, u_int16_t rawsize);
  void dumpFlows();
  inline u_int32_t get_num_hosts()   { return(num_hosts);                };
  inline void      dec_num_hosts()   { num_hosts--;                      };
  void getnDPIStats(NdpiStats *stats);
};

#endif /* _NETWORK_INTERFACE_H_ */
