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

#include "ntop_includes.h"

/* ************************************ */

FlowHash::FlowHash(u_int _num_hashes, u_int _max_hash_size) : GenericHash(_num_hashes, _max_hash_size) {
  ;
};

/* ************************************ */

Flow* FlowHash::find(struct ndpi_iphdr *iph, 
		     struct ndpi_ip6_hdr *ip6,
		     u_int16_t src_port, u_int16_t dst_port, 
		     u_int16_t vlanId, u_int8_t protocol) {

  if(iph) {
    u_int32_t hash = ((iph->saddr+iph->daddr+src_port+dst_port+vlanId+protocol) % num_hashes);
    Flow *head = (Flow*)table[hash];
  
    while(head) {
      if(head->equal(iph->saddr, iph->daddr, src_port, dst_port, vlanId, protocol))
	return(head);
      else
	head = (Flow*)head->next();
    }
  } else if(ip6) {
    u_int32_t src = ip6->ip6_src.__u6_addr.__u6_addr32[0] + ip6->ip6_src.__u6_addr.__u6_addr32[1] + ip6->ip6_src.__u6_addr.__u6_addr32[2] + ip6->ip6_src.__u6_addr.__u6_addr32[3];
    u_int32_t dst = ip6->ip6_dst.__u6_addr.__u6_addr32[0] + ip6->ip6_dst.__u6_addr.__u6_addr32[1] + ip6->ip6_dst.__u6_addr.__u6_addr32[2] + ip6->ip6_dst.__u6_addr.__u6_addr32[3];
    u_int32_t hash = ((src+dst+src_port+dst_port+vlanId+protocol) % num_hashes);
    Flow *head = (Flow*)table[hash];

    while(head) {
      if(head->equal(&ip6->ip6_src, &ip6->ip6_dst, src_port, dst_port, vlanId, protocol))
	return(head);
      else
	head = (Flow*)head->next();
    }
    
  }

  return(NULL);
}
