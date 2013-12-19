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

  /* Purify name */
  for(int i=0; keyname[i] != '\0'; i++)
    if((keyname[i] == '%')
       || (keyname[i] == '"')
       || (keyname[i] == '\''))
      keyname[i] = '_';
  /*
    Set to true if this host can be persistently
    written on disk
  */
  tracked_host = false;
  readStats();
}

/* *************************************** */

StringHost::~StringHost() {
  flushContacts();

  dumpStats(ntop->getPrefs()->get_aggregation_mode() == aggregations_enabled_with_bitmap_dump);
  free(keyname);
}

/* *************************************** */

void StringHost::flushContacts() {
  if(tracked_host) {
    bool _localHost = localHost;

    localHost = true; /* Hack */
    ntop->getRedis()->addIpToDBDump(iface, NULL, keyname);
    dumpContacts(keyname, family_id);
    contacts->purgeAll();
    localHost = _localHost;
  } 
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
  lua_push_int_table_entry(vm, "family", family_id);
  //ndpi_get_proto_name(iface->get_ndpi_struct(), family_id));
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

bool StringHost::addIfMatching(lua_State* vm, char *key) {
  if(strcasestr(host_key(), key)) {
    lua_push_str_table_entry(vm, host_key(), host_key());
    return(true);
  } else
    return(false);
}

/* *************************************** */

char* StringHost::get_string_key(char *buf, u_int buf_len) {
  snprintf(buf, buf_len, "%s", host_key());
  return(buf);
}

