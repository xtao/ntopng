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

#include "ntop_includes.h"

/* *************************************** */

DnsStats::DnsStats() {
  memset(&sent, 0, sizeof(struct dns_stats));
  memset(&rcvd, 0, sizeof(struct dns_stats));
}

/* *************************************** */

void DnsStats::lua(lua_State *vm) {
  lua_newtable(vm);
  lua_push_int_table_entry(vm, "sent_num_queries", sent.num_queries);
  lua_push_int_table_entry(vm, "sent_num_replies_ok", sent.num_replies_ok);
  lua_push_int_table_entry(vm, "sent_num_replies_error", sent.num_replies_error);
  lua_push_int_table_entry(vm, "rcvd_num_queries", rcvd.num_queries);
  lua_push_int_table_entry(vm, "rcvd_num_replies_ok", rcvd.num_replies_ok);
  lua_push_int_table_entry(vm, "rcvd_num_replies_error", rcvd.num_replies_error);
  lua_pushstring(vm, "dns");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

}

/* *************************************** */

char* DnsStats::serialize() {
  json_object *my_object = getJSONObject();
  char *rsp = strdup(json_object_to_json_string(my_object));

  /* Free memory */
  json_object_put(my_object);

  return(rsp);
}

/* ******************************************* */

void DnsStats::deserialize(json_object *o) {
  json_object *obj;

  if(!o) return;

  if(json_object_object_get_ex(o, "sent_num_queries", &obj)) sent.num_queries = json_object_get_int64(obj); else sent.num_queries = 0;
  if(json_object_object_get_ex(o, "sent_num_replies_ok", &obj)) sent.num_replies_ok = json_object_get_int64(obj); else sent.num_replies_ok = 0;
  if(json_object_object_get_ex(o, "sent_num_replies_error", &obj)) sent.num_replies_error = json_object_get_int64(obj); else sent.num_replies_error = 0;
  if(json_object_object_get_ex(o, "rcvd_num_queries", &obj)) rcvd.num_queries = json_object_get_int64(obj); else rcvd.num_queries = 0;
  if(json_object_object_get_ex(o, "rcvd_num_replies_ok", &obj)) rcvd.num_replies_ok = json_object_get_int64(obj); else rcvd.num_replies_ok = 0;
  if(json_object_object_get_ex(o, "rcvd_num_replies_error", &obj)) rcvd.num_replies_error = json_object_get_int64(obj); else rcvd.num_replies_error = 0;
}

/* ******************************************* */

json_object* DnsStats::getJSONObject() {
  json_object *my_object;

  my_object = json_object_new_object();

  if(sent.num_queries > 0) json_object_object_add(my_object, "sent_num_queries", json_object_new_int64(sent.num_queries));
  if(sent.num_replies_ok > 0) json_object_object_add(my_object, "sent_num_replies_ok", json_object_new_int64(sent.num_replies_ok));
  if(sent.num_replies_error > 0) json_object_object_add(my_object, "sent_num_replies_error", json_object_new_int64(sent.num_replies_error));
  if(rcvd.num_queries > 0) json_object_object_add(my_object, "rcvd_num_queries", json_object_new_int64(rcvd.num_queries));
  if(rcvd.num_replies_ok > 0) json_object_object_add(my_object, "rcvd_num_replies_ok", json_object_new_int64(rcvd.num_replies_ok));
  if(rcvd.num_replies_error > 0) json_object_object_add(my_object, "rcvd_num_replies_error", json_object_new_int64(rcvd.num_replies_error));
  
  return(my_object);
}
