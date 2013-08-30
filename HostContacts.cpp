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

HostContacts::HostContacts() {
  memset(clientContacts, 0, sizeof(IPContacts)*MAX_NUM_HOST_CONTACTS);
  memset(serverContacts, 0, sizeof(IPContacts)*MAX_NUM_HOST_CONTACTS);
}

/* *************************************** */

HostContacts::~HostContacts() {
  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(clientContacts[i].host != NULL) delete clientContacts[i].host;
    if(serverContacts[i].host != NULL) delete serverContacts[i].host;
  }
}

/* *************************************** */

void HostContacts::incrIPContacts(IpAddress *peer, IPContacts *contacts, u_int32_t value) {
  int8_t    least_idx = -1;
  u_int32_t least_value = 0;

  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(contacts[i].host == NULL) {
      /* Empty slot */
      contacts[i].host = new IpAddress(peer), contacts[i].num_contacts = value;
      return;
    } else if(contacts[i].host->compare(peer) == 0) {
      contacts[i].num_contacts += value;
      return;
    } else {
      if((least_idx == -1) || (least_value > contacts[i].num_contacts))
	least_idx = i, least_value = contacts[i].num_contacts;
    }
  } /* for */

  /* No room found: let's discard the item with lowest score */
  delete contacts[least_idx].host;
  contacts[least_idx].host = new IpAddress(peer), contacts[least_idx].num_contacts = value;
}

/* *************************************** */

void HostContacts::getIPContacts(lua_State* vm) {
  char buf[64];

  lua_newtable(vm);

  /* client */
  lua_newtable(vm);
  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(clientContacts[i].host != NULL)
      lua_push_int_table_entry(vm,
			       clientContacts[i].host->print(buf, sizeof(buf)),
			       clientContacts[i].num_contacts);
  }
  lua_pushstring(vm, "client");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* server */
  lua_newtable(vm);
  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(serverContacts[i].host != NULL)
      lua_push_int_table_entry(vm,
			       serverContacts[i].host->print(buf, sizeof(buf)),
			       serverContacts[i].num_contacts);
  }
  lua_pushstring(vm, "server");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  lua_pushstring(vm, "contacts");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

char* HostContacts::serialize() {
  json_object *my_object;
  json_object *inner;
  char *rsp;

  my_object = json_object_new_object();

  /* *************************************** */

  inner = json_object_new_object();

  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(clientContacts[i].host != NULL) {
      char buf[64], buf2[32];

      snprintf(buf2, sizeof(buf2), "%u", clientContacts[i].num_contacts);
      json_object_object_add(inner, clientContacts[i].host->print(buf, sizeof(buf)), json_object_new_string(buf2));
    }
  }
  
  json_object_object_add(my_object, "client", inner);
  
  /* *************************************** */

  inner = json_object_new_object();

  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(serverContacts[i].host != NULL) {
      char buf[64], buf2[32];
      
      snprintf(buf2, sizeof(buf2), "%u", serverContacts[i].num_contacts);
      json_object_object_add(inner, serverContacts[i].host->print(buf, sizeof(buf)), json_object_new_string(buf2));      
    }
  }

  json_object_object_add(my_object, "server", inner);

  /* *************************************** */

  rsp = strdup(json_object_to_json_string(my_object));

  // ntop->getTrace()->traceEvent(TRACE_WARNING, "%s", rsp);
  
  /* Free memory */
  json_object_put(my_object);

  return(rsp);
}

/* *************************************** */

void HostContacts::deserialize(json_object *o) {
  json_object *obj;
  IpAddress ip;

  if(!o) return;

  /* Reset all */
  memset(clientContacts, 0, sizeof(IPContacts)*MAX_NUM_HOST_CONTACTS);
  memset(serverContacts, 0, sizeof(IPContacts)*MAX_NUM_HOST_CONTACTS);

  if(json_object_object_get_ex(o, "client", &obj)) {
    struct json_object_iterator it = json_object_iter_begin(obj);
    struct json_object_iterator itEnd = json_object_iter_end(obj);

    while (!json_object_iter_equal(&it, &itEnd)) {
      char *key  = (char*)json_object_iter_peek_name(&it);
      int  value = json_object_get_int(json_object_iter_peek_value(&it));
      
      ip.set_from_string(key);
      incrContact(&ip, true /* client */, value);

      //ntop->getTrace()->traceEvent(TRACE_WARNING, "%s=%d", key, value);

      json_object_iter_next(&it);
    }
  }

  if(json_object_object_get_ex(o, "server", &obj)) {
    struct json_object_iterator it = json_object_iter_begin(obj);
    struct json_object_iterator itEnd = json_object_iter_end(obj);

    while (!json_object_iter_equal(&it, &itEnd)) {
      char *key  = (char*)json_object_iter_peek_name(&it);
      int  value = json_object_get_int(json_object_iter_peek_value(&it));

      ip.set_from_string(key);
      incrContact(&ip, false /* server */, value);
      
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "%s=%d", key, value);

      json_object_iter_next(&it);
    }
  }
}

/* *************************************** */

json_object* HostContacts::getJSONObject() {
  char *s = serialize();
  json_object *o = json_tokener_parse(s);

  free(s);
  return(o);
}
