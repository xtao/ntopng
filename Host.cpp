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

/* *************************************** */

Host::Host() {
  ;
}

/* *************************************** */

Host::Host(u_int32_t _ipv4) {
  set_ipv4(_ipv4);
  initialize();
}

/* *************************************** */

Host::Host(struct in6_addr _ipv6) {
  set_ipv6(_ipv6);
  initialize();
}

/* *************************************** */

Host::~Host() {

}

/* *************************************** */

void Host::initialize() {
  num_uses = 0;

  /* FIX - set ip.localHost */
}

/* *************************************** */

int Host::compare(Host *node) {
  if(ip.ipVersion < node->ip.ipVersion) return(-1); else if(ip.ipVersion > node->ip.ipVersion) return(1);
  
  if(ip.ipVersion == 4)
    return(memcmp(&ip.ipType.ipv4, &node->ip.ipType.ipv4, sizeof(u_int32_t)));
  else
    return(memcmp(&ip.ipType.ipv6, &node->ip.ipType.ipv6, sizeof(struct in6_addr)));
}

/* *************************************** */

void Host::incStats(u_int ndpi_proto, 
		    u_int32_t sent_packets, u_int32_t sent_bytes, 
		    u_int32_t rcvd_packets, u_int32_t rcvd_bytes) {


}

/* *************************************** */
