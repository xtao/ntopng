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

#ifndef _DNS_STATS_H_
#define _DNS_STATS_H_

#include "ntop_includes.h"

struct dns_stats {
  u_int32_t num_queries, num_replies_ok, num_replies_error;    
};

class DnsStats {
 private:
  struct dns_stats sent, rcvd;

 public:
  DnsStats();

  inline void incNumDNSQueriesSent() { sent.num_queries++; };
  inline void incNumDNSQueriesRcvd() { rcvd.num_queries++; };
  inline void incNumDNSResponsesSent(u_int8_t ret_code) { if(ret_code == 0) sent.num_replies_ok++; else sent.num_replies_error++; };
  inline void incNumDNSResponsesRcvd(u_int8_t ret_code) { if(ret_code == 0) rcvd.num_replies_ok++; else rcvd.num_replies_error++; };

  char* serialize();
  void deserialize(json_object *o);
  json_object* getJSONObject();
  void lua(lua_State *vm);
};

#endif /* _STATS_H_ */
