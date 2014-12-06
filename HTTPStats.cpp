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
  memset(&req, 0, sizeof(req));
  memset(&rsp, 0, sizeof(rsp));
}

/* *************************************** */

void HTTPStats::lua(lua_State *vm) {
  lua_newtable(vm);

  lua_push_int_table_entry(vm, "req.total", req.num_get+req.num_get+req.num_post+req.num_head+req.num_put);
  lua_push_int_table_entry(vm, "req.num_get", req.num_get);
  lua_push_int_table_entry(vm, "req.num_post", req.num_post);
  lua_push_int_table_entry(vm, "req.num_head", req.num_head);
  lua_push_int_table_entry(vm, "req.num_put", req.num_put);
  lua_push_int_table_entry(vm, "req.num_other", req.num_other);

  lua_push_int_table_entry(vm, "rsp.total", rsp.num_100x+rsp.num_200x+rsp.num_300x+rsp.num_400x+rsp.num_500x);
  lua_push_int_table_entry(vm, "rsp.num_100x", rsp.num_100x);
  lua_push_int_table_entry(vm, "rsp.num_200x", rsp.num_200x);
  lua_push_int_table_entry(vm, "rsp.num_300x", rsp.num_300x);
  lua_push_int_table_entry(vm, "rsp.num_400x", rsp.num_400x);
  lua_push_int_table_entry(vm, "rsp.num_500x", rsp.num_500x);

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

  memset(&req, 0, sizeof(req)), memset(&rsp, 0, sizeof(rsp));

  if(json_object_object_get_ex(o, "req.num_get", &obj))   req.num_get = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "req.num_post", &obj))  req.num_post = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "req.num_head", &obj))  req.num_head = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "req.num_put", &obj))   req.num_put = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "req.num_other", &obj)) req.num_other = json_object_get_int64(obj);

  if(json_object_object_get_ex(o, "rsp.num_100x", &obj)) rsp.num_100x = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "rsp.num_200x", &obj)) rsp.num_200x = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "rsp.num_300x", &obj)) rsp.num_300x = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "rsp.num_400x", &obj)) rsp.num_400x = json_object_get_int64(obj);
  if(json_object_object_get_ex(o, "rsp.num_100x", &obj)) rsp.num_100x = json_object_get_int64(obj);
}

/* ******************************************* */

json_object* HTTPStats::getJSONObject() {
  json_object *my_object = json_object_new_object();

  if(req.num_get > 0) json_object_object_add(my_object, "req.num_get", json_object_new_int64(req.num_get));
  if(req.num_post > 0) json_object_object_add(my_object, "req.num_post", json_object_new_int64(req.num_post));
  if(req.num_head > 0) json_object_object_add(my_object, "req.num_head", json_object_new_int64(req.num_head));
  if(req.num_put > 0) json_object_object_add(my_object, "req.num_put", json_object_new_int64(req.num_put));
  if(req.num_other > 0) json_object_object_add(my_object, "req.num_other", json_object_new_int64(req.num_other));

  if(rsp.num_100x > 0) json_object_object_add(my_object, "rsp.num_100x", json_object_new_int64(rsp.num_100x));
  if(rsp.num_200x > 0) json_object_object_add(my_object, "rsp.num_200x", json_object_new_int64(rsp.num_200x));
  if(rsp.num_300x > 0) json_object_object_add(my_object, "rsp.num_300x", json_object_new_int64(rsp.num_300x));
  if(rsp.num_400x > 0) json_object_object_add(my_object, "rsp.num_400x", json_object_new_int64(rsp.num_400x));
  if(rsp.num_500x > 0) json_object_object_add(my_object, "rsp.num_500x", json_object_new_int64(rsp.num_500x));

  return(my_object);
}

/* ******************************************* */

void HTTPStats::incRequest(char *method) {
  if(method[0] == 'G') req.num_get++;
  else if((method[0] == 'P') && (method[0] == 'O')) req.num_post++;
  else if(method[0] == 'H') req.num_head++;
  else if((method[0] == 'P') && (method[0] == 'U')) req.num_put++;
  else req.num_other++;
}

/* ******************************************* */

void HTTPStats::incResponse(char *return_code) {
  char *code;

  if(!return_code) return; else code = strchr(return_code, ' ');
  if(!code) return;

  switch(code[1]) {
  case '1': rsp.num_100x++; break;
  case '2': rsp.num_200x++; break;
  case '3': rsp.num_300x++; break;
  case '4': rsp.num_400x++; break;
  case '5': rsp.num_500x++; break;
  }
}
