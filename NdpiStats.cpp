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

NdpiStats::NdpiStats() {
  memset(packets, 0, sizeof(packets)), memset(bytes, 0, sizeof(bytes));
}

/* *************************************** */

void NdpiStats::sumStats(NdpiStats *stats) {
  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    stats->packets[i].sent += packets[i].sent, stats->packets[i].rcvd += packets[i].rcvd;
    stats->bytes[i].sent += bytes[i].sent, stats->bytes[i].rcvd += bytes[i].rcvd;
  }
}

/* *************************************** */

void NdpiStats::print(NetworkInterface *iface) {
  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    if(packets[i].sent || packets[i].rcvd)
      printf("[%s] [pkts: %u/%u][bytes: %u/%u]\n", 
	     iface->get_ndpi_proto_name(i),
	     packets[i].sent, packets[i].rcvd,
	     bytes[i].sent, bytes[i].rcvd);
  }
}

/* *************************************** */

void NdpiStats::dumpToLua(NetworkInterface *iface, lua_State* vm) {
  lua_newtable(vm);

  for(int i=0; i<MAX_NDPI_PROTOS; i++)
    if(packets[i].sent || packets[i].rcvd) {
      lua_pushstring(vm, iface->get_ndpi_proto_name(i));
      lua_pushinteger(vm, packets[i].sent+packets[i].rcvd);
      lua_settable(vm, -3);
  }
}
