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

class Host : public GenericHashEntry {
 private:
  u_int8_t mac_address[6];
  u_int32_t asn;
  char *symbolic_name, *country, *city, *asname, category[8];
  u_int16_t num_uses, vlan_id;
  float latitude, longitude;
  IpAddress *ip;
  NdpiStats *ndpiStats;
  TrafficStats sent, rcvd;
  TrafficStats tcp_sent, tcp_rcvd;
  TrafficStats udp_sent, udp_rcvd;
  TrafficStats icmp_sent, icmp_rcvd;
  TrafficStats other_ip_sent, other_ip_rcvd;
  Mutex *m;
  bool localHost;
  
  void updateLocal();
  void initialize(u_int8_t mac[6], u_int16_t _vlan_id, bool init_all);
  void refreshCategory();

 public:
  Host(NetworkInterface *_iface);
  Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId);
  Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId, IpAddress *_ip);
  ~Host();

  inline void set_ipv4(u_int32_t _ipv4)        { ip->set_ipv4(_ipv4);   }
  inline void set_ipv6(struct ndpi_in6_addr *_ipv6) { ip->set_ipv6(_ipv6); }
  u_int32_t key();
  inline IpAddress* get_ip()                   { return(ip);            }
  inline u_int8_t*  get_mac()                  { return(mac_address);   }
  inline u_int16_t get_vlan_id()               { return(vlan_id);       }
  inline char* get_name()                      { return(symbolic_name); }
  inline char* get_country()                   { return(country);       }
  inline char* get_city()                      { return(city);          }
  inline char* get_category()                  { refreshCategory(); return(category); }
  inline u_int32_t get_asn()                   { return(asn);           }
  inline bool isLocalHost()                    { return(localHost);     }
  char* get_mac(char *buf, u_int buf_len);
  char* get_name(char *buf, u_int buf_len, bool force_resolution_if_not_found);

  void incUses() { num_uses++; }
  void decUses() { num_uses--; }
  
  void incStats(u_int8_t l4_proto, u_int ndpi_proto, u_int32_t sent_packets, u_int64_t sent_bytes, u_int32_t rcvd_packets, u_int64_t rcvd_bytes);

  inline NdpiStats* get_ndpi_stats() { return(ndpiStats); };
  inline bool idle()                 { return(isIdle(HOST_MAX_IDLE)); };
  void lua(lua_State* vm, bool host_details, bool returnHost);
  void resolveHostName();
  void setName(char *name, bool update_categorization);
  int compare(Host *h);
  inline bool equal(IpAddress *_ip)  { return(ip->equal(_ip)); };
  bool isIdle(u_int max_idleness);
};

#endif /* _HOST_H_ */
