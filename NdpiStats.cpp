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
#include <string>

/* *************************************** */

NdpiStats::NdpiStats() {
  memset(counters, 0, sizeof(counters));
}

/* *************************************** */

NdpiStats::~NdpiStats() {
  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    if(counters[i] != NULL)
      free(counters[i]);
  }
}

/* *************************************** */

void NdpiStats::sumStats(NdpiStats *stats) {
  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    if(counters[i] != NULL) {
      if(stats->counters[i] == NULL) {
	if((stats->counters[i] = (ProtoCounter*)calloc(1, sizeof(ProtoCounter))) == NULL) {
	  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Not enough memory");
	  return;
	}
      }

      stats->counters[i]->packets.sent += counters[i]->packets.sent;
      stats->counters[i]->packets.rcvd += counters[i]->packets.rcvd;
      stats->counters[i]->bytes.sent   += counters[i]->bytes.sent;
      stats->counters[i]->bytes.rcvd   += counters[i]->bytes.rcvd;
    }
  }
}

/* *************************************** */

void NdpiStats::print(NetworkInterface *iface) {
  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    if(counters[i] != NULL) {
      if(counters[i]->packets.sent || counters[i]->packets.rcvd)
	printf("[%s] [pkts: %llu/%llu][bytes: %llu/%llu]\n", 
	       iface->get_ndpi_proto_name(i),
	       (long long unsigned) counters[i]->packets.sent, (long long unsigned) counters[i]->packets.rcvd,
	       (long long unsigned) counters[i]->bytes.sent,   (long long unsigned)counters[i]->bytes.rcvd);
    }
  }
}

/* *************************************** */

void NdpiStats::lua(NetworkInterface *iface, lua_State* vm) {
  lua_newtable(vm);
  
  for(int i=0; i<MAX_NDPI_PROTOS; i++)
    if(counters[i] != NULL) {
      if(counters[i]->packets.sent || counters[i]->packets.rcvd) {
	lua_newtable(vm);
	lua_push_int_table_entry(vm, "packets.sent", counters[i]->packets.sent);
	lua_push_int_table_entry(vm, "packets.rcvd", counters[i]->packets.rcvd);
	lua_push_int_table_entry(vm, "bytes.sent", counters[i]->bytes.sent);
	lua_push_int_table_entry(vm, "bytes.rcvd", counters[i]->bytes.rcvd);
	
	lua_pushstring(vm, iface->get_ndpi_proto_name(i)); // Index
	lua_insert(vm, -2);
	lua_settable(vm, -3);
      }
  }

  lua_pushstring(vm, "ndpi");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

void NdpiStats::incStats(u_int proto_id,
			 u_int64_t sent_packets, u_int64_t sent_bytes,
			 u_int64_t rcvd_packets, u_int64_t rcvd_bytes) {
  if(proto_id < (MAX_NDPI_PROTOS)) {
    if(counters[proto_id] == NULL) {
      if((counters[proto_id] = (ProtoCounter*)calloc(1, sizeof(ProtoCounter))) == NULL) {
	ntop->getTrace()->traceEvent(TRACE_NORMAL, "Not enough memory");
	return;
      }
    }

    counters[proto_id]->packets.sent += sent_packets, counters[proto_id]->bytes.sent += sent_bytes;
    counters[proto_id]->packets.rcvd += rcvd_packets, counters[proto_id]->bytes.rcvd += rcvd_bytes;
  }
}

/* *************************************** */

const char* NdpiStats::serialize() {

  return(NULL);
}

/* *************************************** */

void NdpiStats::deserialize(const char *v) {
}

