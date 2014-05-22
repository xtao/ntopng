/*
 *
 * (C) 2013-14 - ntop.org
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

#ifndef _FLOW_H_
#define _FLOW_H_

#include "ntop_includes.h"

class Flow : public GenericHashEntry {
 private:
  Host *cli_host, *srv_host;  
  u_int16_t cli_port, srv_port;
  u_int16_t vlanId;
  u_int8_t protocol, tcp_flags;
  struct ndpi_flow_struct *ndpi_flow;
  bool detection_completed, protocol_processed;
  u_int16_t ndpi_detected_protocol;
  void *cli_id, *srv_id;
  char *json_info;
  struct {
    char *category;
    bool flow_categorized;
  } categorization;

  /* Process Information */
  ProcessInfo *client_proc, *server_proc;

  /* Stats */
  u_int32_t cli2srv_packets, srv2cli_packets;
  u_int64_t cli2srv_bytes, srv2cli_bytes;

  struct {
    char *name;
  } aggregationInfo;

  /* Counter values at last host update */
  struct timeval last_update_time;
  float bytes_thpt;
  ValueTrend bytes_thpt_trend;
  u_int64_t cli2srv_last_packets, srv2cli_last_packets;
  u_int64_t cli2srv_last_bytes, srv2cli_last_bytes,
    prev_cli2srv_last_bytes, prev_srv2cli_last_bytes;  

  char* intoaV4(unsigned int addr, char* buf, u_short bufLen);
  void processLua(lua_State* vm, ProcessInfo *proc, bool client);
  json_object* processJson(ProcessInfo *proc);

  void refresh_process();
  void refresh_process_peer(Host *host, u_int16_t port, bool as_client);

 public:
  Flow(NetworkInterface *_iface,
       u_int16_t _vlanId, u_int8_t _protocol,
       u_int8_t cli_mac[6], IpAddress *_cli_ip, u_int16_t _cli_port,
       u_int8_t srv_mac[6], IpAddress *_srv_ip, u_int16_t _srv_port);
  Flow(NetworkInterface *_iface,
       u_int16_t _vlanId, u_int8_t _protocol, 
       u_int8_t cli_mac[6], IpAddress *_cli_ip, u_int16_t _cli_port,
       u_int8_t srv_mac[6], IpAddress *_srv_ip, u_int16_t _srv_port,
       time_t _first_seen, time_t _last_seen);
  ~Flow();

  char *getDomainCategory();
  void allocFlowMemory();
  void deleteFlowMemory();
  char* serialize();
  inline u_int8_t getTcpFlags() { return(tcp_flags);  };
  u_int32_t getPid(bool client);
  u_int32_t getFatherPid(bool client);
  char* get_username(bool client);
  char* get_proc_name(bool client);
  void updateTcpFlags(time_t when, u_int8_t flags);
  void processDetectedProtocol();
  void setDetectedProtocol(u_int16_t proto_id);
  void setJSONInfo(const char *json);
  bool isFlowPeer(char *numIP);
  void incStats(bool cli2srv_direction, u_int pkt_len);
  void updateActivities();
  void addFlowStats(bool cli2srv_direction, u_int in_pkts, u_int in_bytes, u_int out_pkts, u_int out_bytes, time_t last_seen);
  inline bool isDetectionCompleted()              { return(detection_completed);             };
  inline struct ndpi_flow_struct* get_ndpi_flow() { return(ndpi_flow);                       };
  inline void* get_cli_id()                       { return(cli_id);                          };
  inline void* get_srv_id()                       { return(srv_id);                          };
  inline u_int32_t get_cli_ipv4()                 { return(cli_host->get_ip()->get_ipv4());  };
  inline u_int32_t get_srv_ipv4()                 { return(srv_host->get_ip()->get_ipv4());  };
  inline u_int16_t get_cli_port()                 { return(ntohs(cli_port));                 };
  inline u_int16_t get_srv_port()                 { return(ntohs(srv_port));                 };
  inline u_int16_t get_vlan_id()                  { return(vlanId);                          };
  inline u_int8_t  get_protocol()                 { return(protocol);                        };
  inline u_int64_t get_bytes()                    { return(cli2srv_bytes+srv2cli_bytes);     };
  inline u_int64_t get_packets()                  { return(cli2srv_packets+srv2cli_packets); };
  inline char* get_protocol_name()                { return(Utils::l4proto2name(protocol));   };
  inline u_int16_t get_detected_protocol()        { return(ndpi_detected_protocol);          };
  inline char* get_detected_protocol_name()       { return(ndpi_get_proto_name(iface->get_ndpi_struct(), ndpi_detected_protocol)); };
  inline Host* get_cli_host()                     { return(cli_host);                        };
  inline Host* get_srv_host()                     { return(srv_host);                        };
  inline char* get_json_info()			  { return(json_info);                       };
  u_int64_t get_current_bytes_cli2srv();
  u_int64_t get_current_bytes_srv2cli();
  void aggregateInfo(char *name, u_int16_t ndpi_proto_id,
		     AggregationType mode, bool aggregation_to_track);
  void handle_process(ProcessInfo *pinfo, bool client_process);
  bool idle();
  int compare(Flow *fb);
  char* print(char *buf, u_int buf_len);
  void update_hosts_stats(struct timeval *tv);
  void print_peers(lua_State* vm, bool verbose);
  u_int32_t key();
  void lua(lua_State* vm, bool detailed_dump);
  bool equal(IpAddress *_cli_ip, IpAddress *_srv_ip,
	     u_int16_t _cli_port, u_int16_t _srv_port,
	     u_int16_t _vlanId, u_int8_t _protocol,
	     bool *src2srv_direction);
  void sumStats(NdpiStats *stats);
  void guessProtocol();
};

#endif /* _FLOW_H_ */
