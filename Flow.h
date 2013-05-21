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

#ifndef _FLOW_H_
#define _FLOW_H_

#include "ntop_includes.h"

class Flow : public HashEntry {
 private:
  Host *src_host, *dst_host;
  u_int16_t src_port, dst_port;
  u_int16_t vlanId;
  u_int8_t protocol;
  struct ndpi_flow_struct *ndpi_flow;
  bool detection_completed;
  u_int16_t detected_protocol;
  void *src_id, *dst_id;

  /* Stats */
  u_int32_t cli2srv_packets, cli2srv_bytes, srv2cli_packets, srv2cli_bytes;
  u_int32_t cli2srv_last_packets, cli2srv_last_bytes, srv2cli_last_packets, srv2cli_last_bytes; /* Counter values at last host update */

  void deleteFlowMemory();
  char* ipProto2Name(u_short proto_id);
  char* intoaV4(unsigned int addr, char* buf, u_short bufLen);

 public:
  Flow(NetworkInterface *_iface,
       u_int16_t _vlanId, u_int8_t _protocol,
       u_int8_t src_mac[6], u_int32_t _src_ipv4, struct ndpi_in6_addr *_src_ipv6, u_int16_t _src_port,
       u_int8_t dst_mac[6], u_int32_t _dst_ipv4, struct ndpi_in6_addr *_dst_ipv6, u_int16_t _dst_port);
  ~Flow();

  void allocFlowMemory();
  void setDetectedProtocol(u_int16_t proto_id, u_int8_t l4_proto);
  inline void incStats(bool cli2srv_direction, u_int pkt_len) { updateSeen(); if(cli2srv_direction) cli2srv_packets++, cli2srv_bytes += pkt_len; else srv2cli_packets++, srv2cli_bytes += pkt_len; };
  inline bool isDetectionCompleted()  { return(detection_completed); };
  inline struct ndpi_flow_struct* get_ndpi_flow() { return(ndpi_flow); };
  inline void* get_src_id()                       { return(src_id);    };
  inline void* get_dst_id()                       { return(dst_id);    };
  inline u_int32_t get_src_ipv4()                 { return(src_host->get_ip()->get_ipv4());  };
  inline u_int32_t get_dst_ipv4()                 { return(dst_host->get_ip()->get_ipv4());  };
  inline u_int16_t get_src_port()                 { return(src_port);  };
  inline u_int16_t get_dst_port()                 { return(dst_port);  };
  inline u_int16_t get_vlan_id()                  { return(vlanId);    };
  inline u_int8_t  get_protocol()                 { return(protocol);  };
  inline char* get_protocol_name()                { return(Utils::l4proto2name(protocol)); };
  inline u_int16_t get_detected_protocol()        { return(detected_protocol); };
  inline char* get_detected_protocol_name()       { return(ndpi_get_proto_name(iface->get_ndpi_struct(), detected_protocol)); }
  inline Host* get_src_host()                     { return(src_host); };
  inline Host* get_dst_host()                     { return(dst_host); };
  inline bool idle()                              { return(isIdle(FLOW_MAX_IDLE)); };
  int compare(Flow *fb);
  void print();
  void update_hosts_stats();
  void print_peers(lua_State* vm);
  inline u_int32_t key()                          { return(src_host->key()+dst_host->key()+src_port+dst_port+vlanId+protocol); }
  void lua(lua_State* vm, bool detailed_dump);
  bool equal(u_int32_t _src_ip, u_int32_t _dst_ip,
	     u_int16_t _src_port, u_int16_t _dst_port,
	     u_int16_t _vlanId, u_int8_t _protocol,
	     bool *src2dst_direction);
  bool equal(struct ndpi_in6_addr *ip6_src, struct ndpi_in6_addr *ip6_dst,
	     u_int16_t _src_port, u_int16_t _dst_port,
	     u_int16_t _vlanId, u_int8_t _protocol,
	     bool *src2dst_direction);
};

#endif /* _FLOW_H_ */
