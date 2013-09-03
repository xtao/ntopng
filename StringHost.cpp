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

StringHost::StringHost(NetworkInterface *_iface, char *_key, 
		       u_int16_t _family_id) : GenericHost(_iface) {
  keyname = strdup(_key), family_id = _family_id;
}

/* *************************************** */

StringHost::~StringHost() {
  free(keyname);
}

/* *************************************** */

bool StringHost::idle() {
  return(isIdle(ntop->getPrefs()->get_host_max_idle())); 
};

/* *************************************** */

void StringHost::lua(lua_State* vm, bool returnHost) {
  lua_newtable(vm);

  lua_push_str_table_entry(vm, "name", keyname);

  lua_push_int_table_entry(vm, "bytes.sent", sent.getNumBytes());
  lua_push_int_table_entry(vm, "bytes.rcvd", rcvd.getNumBytes());
  lua_push_int_table_entry(vm, "pkts.sent", sent.getNumPkts());
  lua_push_int_table_entry(vm, "pkts.rcvd", rcvd.getNumPkts());
  lua_push_int_table_entry(vm, "seen.first", first_seen);
  lua_push_int_table_entry(vm, "seen.last", last_seen);
  lua_push_int_table_entry(vm, "duration", get_duration());
  lua_push_float_table_entry(vm, "throughput", bytes_thpt);
  lua_push_int_table_entry(vm, "throughput_trend", getThptTrend());

  if(ndpiStats) ndpiStats->lua(iface, vm);
  getHostContacts(vm);

  if(returnHost) {
    lua_pushstring(vm, keyname);
    lua_insert(vm, -2);
    lua_settable(vm, -3);
  }
}

/* *************************************** */

char* StringHost::get_string_key(char *buf, u_int buf_len) {
  snprintf(buf, buf_len, "%s", host_key());
  return(buf);
}

