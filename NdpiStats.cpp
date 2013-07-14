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
      printf("[%s] [pkts: %llu/%llu][bytes: %llu/%llu]\n", 
	     iface->get_ndpi_proto_name(i),
	     (long long unsigned) packets[i].sent, (long long unsigned) packets[i].rcvd,
	     (long long unsigned) bytes[i].sent,   (long long unsigned)bytes[i].rcvd);
  }
}

/* *************************************** */

void NdpiStats::lua(NetworkInterface *iface, lua_State* vm) {
  lua_newtable(vm);
  
  for(int i=0; i<MAX_NDPI_PROTOS; i++)
    if(packets[i].sent || packets[i].rcvd) {
      lua_newtable(vm);
      lua_push_int_table_entry(vm, "packets.sent", packets[i].sent);
      lua_push_int_table_entry(vm, "packets.rcvd", packets[i].rcvd);
      lua_push_int_table_entry(vm, "bytes.sent", bytes[i].sent);
      lua_push_int_table_entry(vm, "bytes.rcvd", bytes[i].rcvd);

      lua_pushstring(vm, iface->get_ndpi_proto_name(i)); // Index
      lua_insert(vm, -2);
      lua_settable(vm, -3);
  }

  lua_pushstring(vm, "ndpi");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

const char* NdpiStats::serialize() {
  std::string s("{");
  bool first = true;

  for(int i=0; i<MAX_NDPI_PROTOS; i++) {
    if(packets[i].sent || packets[i].rcvd) {
      char tmp[128];

      snprintf(tmp, sizeof(tmp), "\"%d\":[%llu,%llu,%llu,%llu]", 
	       i, 
	       packets[i].sent, packets[i].rcvd,
	       bytes[i].sent, bytes[i].rcvd);

      if(!first) s.append(",");
      s.append(tmp);
      first = false;
    }
  }
  
  s.append("}");
  return(strdup(s.c_str()));
}

/* *************************************** */

void NdpiStats::deserialize(const char *v) {
  json_settings settings;
  char error[256];
  json_value *value;

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s", v);
  
  /* Reset all values */
  memset(packets, 0, sizeof(packets)), memset(bytes, 0, sizeof(bytes));

  memset(&settings, 0, sizeof (json_settings));
  if((value = json_parse_ex(&settings, v, strlen(v), error)) != NULL) {
    if(value->type == json_object) {
      for(u_int i=0; i<value->u.object.length; i++) {
	u_int id = atoi(value->u.object.values[i].name);

	if(id < MAX_NDPI_PROTOS) {
	  if((value->u.object.values[i].value->type == json_array)
	     && (value->u.object.values[i].value->u.array.length == 4)) {
	    u_int64_t v;

	    if(value->u.object.values[i].value->u.array.values[0]->type == json_integer) {
	      v = (u_int64_t)value->u.object.values[i].value->u.array.values[0]->u.integer;
	      packets[id].sent = v;
	    }

	    if(value->u.object.values[i].value->u.array.values[1]->type == json_integer) {
	      v = (u_int64_t)value->u.object.values[i].value->u.array.values[1]->u.integer;
	      packets[id].rcvd = v;
	    }

	    if(value->u.object.values[i].value->u.array.values[2]->type == json_integer) {
	      v = (u_int64_t)value->u.object.values[i].value->u.array.values[2]->u.integer;
	      bytes[id].sent = v;
	    }

	    if(value->u.object.values[i].value->u.array.values[3]->type == json_integer) {	      
	      v = (u_int64_t)value->u.object.values[i].value->u.array.values[3]->u.integer;
	      bytes[id].rcvd = v;
	    }
	  }
	}
      }
    }

    json_value_free(value);
  }
}

