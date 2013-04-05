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


class Flow {
 private:
  // FIX - Add IPv6 support
  u_int32_t lower_ip, upper_ip;
  u_int16_t lower_port, upper_port;
  u_int16_t vlanId;
  u_int8_t protocol;
  struct ndpi_flow_struct *ndpi_flow;
  bool detection_completed;
  u_int16_t detected_protocol;
  void *src_id, *dst_id;
  /* Stats */
  u_int32_t packets, bytes;

  void deleteFlowMemory();
  char* ipProto2Name(u_short proto_id);
  char* intoaV4(unsigned int addr, char* buf, u_short bufLen);

 public:
  Flow(u_int16_t _vlanId, u_int8_t _protocol, 
       u_int32_t _lower_ip, u_int16_t _lower_port,
       u_int32_t _upper_ip, u_int16_t _upper_port);
  ~Flow();

  void allocFlowMemory();
  void setDetectedProtocol(u_int16_t proto_id, u_int8_t l4_proto);
  inline void incStats(u_int pkt_len) { packets++, bytes += pkt_len; };  
  inline bool isDetectionCompleted()  { return(detection_completed); };
  inline struct ndpi_flow_struct* get_ndpi_flow() { return(ndpi_flow); };
  inline void* get_src_id()                       { return(src_id);    };
  inline void* get_dst_id()                       { return(dst_id);    };
  inline u_int16_t get_vlan_id()                  { return(vlanId);    };  
  inline u_int16_t get_detected_protocol()        { return(detected_protocol); };
  int compare(Flow *fb);
  void print();
};

#endif /* _FLOW_H_ */
