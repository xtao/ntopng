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

HTTPStats::HTTPStats() {
  memset(&query, 0, sizeof(query));
  memset(&response, 0, sizeof(response));
}

/* *************************************** */

void HTTPStats::lua(lua_State *vm) {
  lua_newtable(vm);

  lua_push_int_table_entry(vm, "query.total", query.num_get+query.num_get+query.num_post+query.num_head+query.num_put);
  lua_push_int_table_entry(vm, "query.num_get", query.num_get);
  lua_push_int_table_entry(vm, "query.num_post", query.num_post);
  lua_push_int_table_entry(vm, "query.num_head", query.num_head);
  lua_push_int_table_entry(vm, "query.num_put", query.num_put);
  lua_push_int_table_entry(vm, "query.num_other", query.num_other);

  lua_push_int_table_entry(vm, "response.total", response.num_1xx+response.num_2xx+response.num_3xx+response.num_4xx+response.num_5xx);
  lua_push_int_table_entry(vm, "response.num_1xx", response.num_1xx);
  lua_push_int_table_entry(vm, "response.num_2xx", response.num_2xx);
  lua_push_int_table_entry(vm, "response.num_3xx", response.num_3xx);
  lua_push_int_table_entry(vm, "response.num_4xx", response.num_4xx);
  lua_push_int_table_entry(vm, "response.num_5xx", response.num_5xx);

  lua_pushstring(vm, "http");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

char* HTTPStats::serialize() {
  json_object *my_object = getJSONObject();
  char *rsp = strdup(json_object_to_json_string(my_object));

  /* Free memory */
  json_object_put(my_object);

  return(rsp);
}

/* ******************************************* */

void HTTPStats::deserialize(json_object *o) {
  json_object *obj;

  if(!o) return;

  memset(&query, 0, sizeof(query)), memset(&response, 0, sizeof(response));

  if(json_object_object_get_ex(o, "query.num_get", &obj))   query.num_get = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "query.num_post", &obj))  query.num_post = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "query.num_head", &obj))  query.num_head = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "query.num_put", &obj))   query.num_put = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "query.num_other", &obj)) query.num_other = json_object_get_int64(obj);

  if(json_object_object_get_ex(o, "response.num_1xx", &obj)) response.num_1xx = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "response.num_2xx", &obj)) response.num_2xx = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "response.num_3xx", &obj)) response.num_3xx = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "response.num_4xx", &obj)) response.num_4xx = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "response.num_1xx", &obj)) response.num_1xx = json_object_get_int64(obj);
}

/* ******************************************* */

json_object* HTTPStats::getJSONObject() {
  json_object *my_object = json_object_new_object();

  if(query.num_get > 0) json_object_object_add(my_object, "query.num_get", json_object_new_int64(query.num_get));
  if(query.num_post > 0) json_object_object_add(my_object, "query.num_post", json_object_new_int64(query.num_post));
  if(query.num_head > 0) json_object_object_add(my_object, "query.num_head", json_object_new_int64(query.num_head));
  if(query.num_put > 0) json_object_object_add(my_object, "query.num_put", json_object_new_int64(query.num_put));
  if(query.num_other > 0) json_object_object_add(my_object, "query.num_other", json_object_new_int64(query.num_other));

  if(response.num_1xx > 0) json_object_object_add(my_object, "response.num_1xx", json_object_new_int64(response.num_1xx));
  if(response.num_2xx > 0) json_object_object_add(my_object, "response.num_2xx", json_object_new_int64(response.num_2xx));
  if(response.num_3xx > 0) json_object_object_add(my_object, "response.num_3xx", json_object_new_int64(response.num_3xx));
  if(response.num_4xx > 0) json_object_object_add(my_object, "response.num_4xx", json_object_new_int64(response.num_4xx));
  if(response.num_5xx > 0) json_object_object_add(my_object, "response.num_5xx", json_object_new_int64(response.num_5xx));

  return(my_object);
}

/* ******************************************* */

void HTTPStats::incRequest(char *method) {
  if(method[0] == 'G') query.num_get++;
  else if((method[0] == 'P') && (method[0] == 'O')) query.num_post++;
  else if(method[0] == 'H') query.num_head++;
  else if((method[0] == 'P') && (method[0] == 'U')) query.num_put++;
  else query.num_other++;
}

/* ******************************************* */

void HTTPStats::incResponse(char *return_code) {
  char *code;

  if(!return_code) return; else code = strchr(return_code, ' ');
  if(!code) return;

  switch(code[1]) {
  case '1': response.num_1xx++; break;
  case '2': response.num_2xx++; break;
  case '3': response.num_3xx++; break;
  case '4': response.num_4xx++; break;
  case '5': response.num_5xx++; break;
  }
}
