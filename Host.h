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

#ifndef _HOST_H_
#define _HOST_H_

#include "ntop_includes.h"

class Host : public GenericHost {
 private:
  u_int8_t mac_address[6];
  u_int32_t asn;
  char *symbolic_name, *alternate_name, *country, *city, *asname, category[8], os[16];
  u_int16_t num_uses, vlan_id;
  float latitude, longitude;
  IpAddress *ip;
  Mutex *m;
  AlertCounter *syn_flood_alert;
  TrafficStats tcp_sent, tcp_rcvd;
  TrafficStats udp_sent, udp_rcvd;
  TrafficStats icmp_sent, icmp_rcvd;
  TrafficStats other_ip_sent, other_ip_rcvd;
  PacketStats sent_stats, recv_stats;
  u_int32_t num_flows_as_client, num_flows_as_server;
  DnsStats *dns;
  EppStats *epp;

  void updateLocal();
  void initialize(u_int8_t mac[6], u_int16_t _vlan_id, bool init_all);
  void refreshCategory();
  void read_alternate_name();
  void save_alternate_name();
  void computeHostSerial();
  inline void allocDNS() { if(dns == NULL) dns = new DnsStats(); }
  inline void allocEPP() { if(epp == NULL) epp = new EppStats(); }

 public:
  Host(NetworkInterface *_iface);
  Host(NetworkInterface *_iface, char *ipAddress);
  Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId);
  Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId, IpAddress *_ip);
  ~Host();

  inline PacketStats* get_sent_stats() { return(&sent_stats); };
  inline PacketStats* get_recv_stats() { return(&recv_stats); };
  inline void set_ipv4(u_int32_t _ipv4)             { ip->set_ipv4(_ipv4); };
  inline void set_ipv6(struct ndpi_in6_addr *_ipv6) { ip->set_ipv6(_ipv6); };
  u_int32_t key();
  char* getJSON();
  inline void setOS(char *_os)                 { if(os[0] == '\0') snprintf(os, sizeof(os), "%s", _os); }
  inline IpAddress* get_ip()                   { return(ip);            }
  void set_mac(char *m);
  void set_alternate_name(char *name);
  inline u_int8_t*  get_mac()                  { return(mac_address);   }
  inline char* get_os()                        { return(os);            }
  inline u_int16_t get_vlan_id()               { return(vlan_id);       }
  inline char* get_name()                      { return(symbolic_name); }
  inline char* get_alternate_name()            { return(alternate_name);}
  inline char* get_country()                   { return(country);       }
  inline char* get_city()                      { return(city);          }
  inline char* get_category()                  { refreshCategory(); return(category); }
  inline u_int32_t get_asn()                   { return(asn);           }
  inline bool isPrivateHost()                  { return((ip && ip->isPrivateAddress()) ? true : false); }
  inline float get_latitude()                  { return(latitude);      }
  inline float get_longitude()                 { return(longitude);     }
  char* get_mac(char *buf, u_int buf_len);
  char* get_name(char *buf, u_int buf_len, bool force_resolution_if_not_found);
  char* get_string_key(char *buf, u_int buf_len);
  void incUses() { num_uses++; }
  void decUses() { num_uses--; }
  bool idle();
  void lua(lua_State* vm, bool host_details, bool verbose, bool returnHost);
  void resolveHostName();
  void setName(char *name, bool update_categorization);
  int compare(Host *h);
  inline bool equal(IpAddress *_ip)  { return(ip->equal(_ip)); };
  bool isIdle(u_int max_idleness);
  void incrContact(Host *peer, bool contacted_peer_as_client);
  void incStats(u_int8_t l4_proto, u_int ndpi_proto, 
		u_int64_t sent_packets, u_int64_t sent_bytes,
		u_int64_t rcvd_packets, u_int64_t rcvd_bytes);

  char* serialize();
  bool deserialize(char *json_str);
  void flushContacts(bool freeHost);
  bool addIfMatching(lua_State* vm, char *key);
  void updateSynFlags(time_t when, u_int8_t flags, Flow *f);

  inline void incNumFlows(bool as_client) { if(as_client) num_flows_as_client++; else num_flows_as_server++; };

  inline void incNumDNSQueriesSent(u_int16_t query_type) { allocDNS(); if(dns) dns->incNumDNSQueriesSent(query_type); };
  inline void incNumDNSQueriesRcvd(u_int16_t query_type) { allocDNS(); if(dns) dns->incNumDNSQueriesRcvd(query_type); };
  inline void incNumDNSResponsesSent(u_int32_t ret_code)  { allocDNS(); if(dns) dns->incNumDNSResponsesSent(ret_code); };
  inline void incNumDNSResponsesRcvd(u_int32_t ret_code)  { allocDNS(); if(dns) dns->incNumDNSResponsesRcvd(ret_code); };

  inline void incNumEPPQueriesSent(u_int16_t query_type) { allocEPP(); if(epp) epp->incNumEPPQueriesSent(query_type); };
  inline void incNumEPPQueriesRcvd(u_int16_t query_type) { allocEPP(); if(epp) epp->incNumEPPQueriesRcvd(query_type); };
  inline void incNumEPPResponsesSent(u_int32_t ret_code)  { allocEPP(); if(epp) epp->incNumEPPResponsesSent(ret_code); };
  inline void incNumEPPResponsesRcvd(u_int32_t ret_code)  { allocEPP(); if(epp) epp->incNumEPPResponsesRcvd(ret_code); };
};

#endif /* _HOST_H_ */
