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

typedef struct ipAddress {
  u_int8_t ipVersion:3 /* Either 4 or 6 */, 
    localHost:1,
    notUsed:4 /* Future use */;

  union {
    struct in6_addr ipv6;
    u_int32_t ipv4; /* Host byte code */
  } ipType;
} IpAddress;


class Host {
 private:
  u_int16_t num_uses;
  IpAddress ip;
  NdpiStats ndpiStats;

  void initialize();

 public:
  Host();
  Host(u_int32_t _ipv4);
  Host(struct in6_addr _ipv6);
  ~Host();

  inline void set_ipv4(u_int32_t _ipv4)       { ip.ipVersion = 4, ip.ipType.ipv4 = _ipv4; }
  inline void set_ipv6(struct in6_addr _ipv6) { ip.ipVersion = 6, memcpy(&ip.ipType.ipv6,  &_ipv6, sizeof(_ipv6)); }

  void incUses() { num_uses++; }
  void decUses() { num_uses--; }

  void incStats(u_int ndpi_proto, u_int32_t sent_packets, u_int32_t sent_bytes, u_int32_t rcvd_packets, u_int32_t rcvd_bytes);
  int compare(Host *node);
  inline NdpiStats* get_ndpi_stats() { return(&ndpiStats); };
};

#endif /* _HOST_H_ */
