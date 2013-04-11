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

#ifndef _HOST_H_
#define _HOST_H_

#include "ntop_includes.h"

class Host : public HashEntry {
 private:
  u_int16_t num_uses;
  IpAddress *ip;
  NdpiStats ndpiStats;
  TrafficStats sent, rcvd;

  void initialize();

 public:
  Host(NetworkInterface *_iface);
  Host(NetworkInterface *_iface, u_int32_t _ipv4);
  Host(NetworkInterface *_iface, struct in6_addr _ipv6);
  ~Host();

  inline void set_ipv4(u_int32_t _ipv4)       { ip->set_ipv4(_ipv4); }
  inline void set_ipv6(struct in6_addr _ipv6) { ip->set_ipv6(_ipv6); }
  inline u_int32_t key()                      { return(ip->key());   }
  inline IpAddress* get_ip()                  { return(ip);          }

  void incUses() { num_uses++; }
  void decUses() { num_uses--; }

  inline void incStats(u_int ndpi_proto, u_int32_t sent_packets, u_int32_t sent_bytes, u_int32_t rcvd_packets, u_int32_t rcvd_bytes) { 
    if(sent_packets || rcvd_packets) {
      sent.incStats(sent_packets, sent_bytes), rcvd.incStats(rcvd_packets, rcvd_bytes);
      ndpiStats.incStats(ndpi_proto, sent_packets, sent_bytes, rcvd_packets, rcvd_bytes);      
      updateSeen();
    }
  }

  inline int compare(Host *node)     { return(ip->compare(node->ip)); };
  inline NdpiStats* get_ndpi_stats() { return(&ndpiStats);            };
  bool isIdle(u_int max_idleness);
  void dumpKeyToLua(lua_State* vm);
};

#endif /* _HOST_H_ */
