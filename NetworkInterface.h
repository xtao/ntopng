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

class Flow;
class FlowHash;
class Host;
class HostHash;

typedef struct ether80211q {
  u_int16_t vlanId;
  u_int16_t protoType;
} Ether80211q;

class NetworkInterface {
 protected:
  char *ifname;
  EthStats ethStats;
  int pcap_datalink_type;
  pthread_t pollLoop;
  int cpu_affinity;
  NdpiStats ndpiStats;
  PacketStats pktStats;
  FlowHash *flows_hash;
  /* Hosts */
  HostHash *hosts_hash;
  /* String hash (Aggregation) */
  StringHash *strings_hash;

  struct ndpi_detection_module_struct *ndpi_struct;
  time_t last_pkt_rcvd, next_idle_flow_purge, next_idle_host_purge, next_idle_aggregated_host_purge;
  bool running;

  void deleteDataStructures();
  Flow* getFlow(u_int8_t *src_eth, u_int8_t *dst_eth, u_int16_t vlan_id,
  		IpAddress *src_ip, IpAddress *dst_ip,
  		u_int16_t src_port, u_int16_t dst_port,
		u_int8_t l4_proto,
		bool *src2dst_direction,
		time_t first_seen, time_t last_seen);
  bool isNumber(const char *str);
  bool validInterface(char *name);

 public:
  NetworkInterface();
  NetworkInterface(const char *name);
  virtual ~NetworkInterface();

  virtual void startPacketPolling();
  virtual void shutdown();
  virtual u_int getNumDrops()                  { return(0);     };
  virtual char *getScriptName()                { return NULL;   }
  virtual char *getEndpoint()                  { return NULL;   };
  virtual bool set_packet_filter(char *filter) { return(false); };
  virtual void incrDrops(u_int32_t num)        { ; }
  inline virtual const char* get_type()      { return("unknown"); }
  inline virtual bool is_ndpi_enabled()      { return(true); }
  inline u_int  getNumnDPIProtocols()        { return(ndpi_get_num_supported_protocols(ndpi_struct)); };
  inline time_t getTimeLastPktRcvd()         { return(last_pkt_rcvd); };
  inline void  setTimeLastPktRcvd(time_t t)  { last_pkt_rcvd = t; };
  inline char* get_ndpi_proto_name(u_int id) { return(ndpi_get_proto_name(ndpi_struct, id));   };
  inline u_int get_flow_size()         { return(ndpi_detection_get_sizeof_ndpi_flow_struct()); };
  inline u_int get_size_id()           { return(ndpi_detection_get_sizeof_ndpi_id_struct());   };
  inline char* get_name()              { return(ifname);                                       };
  inline struct ndpi_detection_module_struct* get_ndpi_struct() { return(ndpi_struct);         };

  inline void incStats(u_int16_t eth_proto, u_int16_t ndpi_proto, u_int pkt_len) { 
    ethStats.incStats(eth_proto, 1, pkt_len);
    ndpiStats.incStats(ndpi_proto, 0, 0, 1, pkt_len); 
    pktStats.incStats(pkt_len);
  };
  inline void addFlowStats(time_t last, u_int pkts, u_int bytes) { if (last > last_pkt_rcvd) last_pkt_rcvd = last; ethStats.incStats(0, pkts, bytes); }
  inline EthStats* getStats()      { return(&ethStats);          };
  inline int get_datalink()        { return(pcap_datalink_type); };
  inline int isRunning()	   { return running; };
  inline void set_cpu_affinity(int core_id) { cpu_affinity = core_id; if (running) Utils::setThreadAffinity(pollLoop, cpu_affinity); };
  bool restoreHost(char *host_ip);
  void printAvailableInterfaces(bool printHelp, int idx, char *ifname, u_int ifname_len);
  void findFlowHosts(u_int16_t vlan_id,
		     u_int8_t src_mac[6], IpAddress *_src_ip, Host **src, 
		     u_int8_t dst_mac[6], IpAddress *_dst_ip, Host **dst);
  Flow* findFlowByKey(u_int32_t key);

  void packet_dissector(const struct pcap_pkthdr *h, const u_char *packet);
  void packet_processing(const u_int32_t when,
			 const u_int64_t time,
			 struct ndpi_ethhdr *eth,
			 u_int16_t vlan_id,
			 struct ndpi_iphdr *iph,
			 struct ndpi_ip6_hdr *ip6,
			 u_int16_t ipsize, u_int16_t rawsize);
  void flow_processing(u_int8_t *src_eth, u_int8_t *dst_eth,
  		       IpAddress *src_ip, IpAddress *dst_ip,
		       u_int16_t src_port, u_int16_t dst_port,
		       u_int16_t vlan_id,
		       u_int16_t proto_id,
		       u_int8_t l4_proto, u_int8_t tcp_flags,
		       u_int in_pkts, u_int in_bytes,
		       u_int out_pkts, u_int out_bytes,
		       u_int first_switched, u_int last_switched,
		       char *additional_fields_json);
  void dumpFlows();
  void getnDPIStats(NdpiStats *stats);
  void updateHostStats();
  void lua(lua_State* v);
  void getActiveHostsList(lua_State* v, bool host_details);
  void getActiveFlowsList(lua_State* v);
  void getFlowPeersList(lua_State* vm, char *numIP);

  void purgeIdle(time_t when);
  u_int purgeIdleFlows();
  u_int purgeIdleHosts();
  u_int purgeIdleAggregatedHosts();

  u_int getNumFlows();
  u_int getNumHosts();
  inline u_int64_t getNumPackets() { return(ethStats.getNumPackets());  };
  inline u_int64_t getNumBytes()   { return(ethStats.getNumBytes());    };

  void runHousekeepingTasks();
  Host* findHostByMac(u_int8_t mac[6], u_int16_t vlanId,
		      bool createIfNotPresent);
  Host* getHost(char *host_ip, u_int16_t vlan_id);
  bool getHostInfo(lua_State* vm, char *host_ip, u_int16_t vlan_id);
  void getActiveAggregatedHostsList(lua_State* vm);
  bool getAggregatedHostInfo(lua_State* vm, char *host_ip);
  bool getAggregationsForHost(lua_State* vm, char *host_ip);
  StringHost* findHostByString(char *keyname, u_int16_t family_id, 
			       bool createIfNotPresent);  
  inline u_int getNumAggregatedHosts() { return(strings_hash->getNumEntries()); }
};

#endif /* _NETWORK_INTERFACE_H_ */
