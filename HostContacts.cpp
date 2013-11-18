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

bool HostContacts::dumpHostToDB(IpAddress *host, LocationPolicy policy) {
  bool do_dump = false;
  
  switch(policy) {
  case location_local_only:
    if(host->isLocalHost()) do_dump = true;
    break;
  case location_remote_only:
    if(!host->isLocalHost()) do_dump = true;
    break;
  case location_all:
    do_dump = true;
    break;
  case location_none:
    do_dump = false;
    break;
  }

  return(do_dump);
}

/* *************************************** */

void HostContacts::incrIPContacts(NetworkInterface *iface, GenericHost *host, 
				  IpAddress *peer,
				  IPContacts *contacts, u_int32_t value) {
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
  if(dumpHostToDB(contacts[least_idx].host, 
		  ntop->getPrefs()->get_dump_hosts_to_db_policy())) {
    char dump_path[MAX_PATH], daybuf[64];
    char key[128];
    time_t when = time(NULL);
    
    strftime(daybuf, sizeof(daybuf), "%y/%m/%d", localtime(&when));
    snprintf(dump_path, sizeof(dump_path), "%s/%s/host_contacts/%s",
	     ntop->get_working_dir(), iface->get_name(), daybuf);
    ntop->fixPath(dump_path);
   
    dbDump(dump_path, host->get_string_key(key, sizeof(key)), HOST_FAMILY_ID);
  }

  delete contacts[least_idx].host;
  contacts[least_idx].host = new IpAddress(peer), 
    contacts[least_idx].num_contacts = value;
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

void HostContacts::deserialize(NetworkInterface *iface, GenericHost *h, json_object *o) {
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
      incrContact(iface, h, &ip, true /* client */, value);

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
      incrContact(iface, h, &ip, false /* server */, value);

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

/* *************************************** */

u_int HostContacts::get_num_contacts_by(IpAddress* host_ip) {
  u_int num = 0;

  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(clientContacts[i].num_contacts == 0) break;

    if(clientContacts[i].host->equal(host_ip))
      num += clientContacts[i].num_contacts;
  }

  return(num);
}

/* *************************************** */

#define ifdot(a) ((a == '.') ? '_' : a)

void HostContacts::dbDump(char *path, char *key, u_int16_t family_id) {
  char buf[64], full_path[MAX_PATH], alt_full_path[MAX_PATH];

  snprintf(full_path, sizeof(full_path), "%s/%c/%c|%s", path, key[0], ifdot(key[1]), key);
  ntop->fixPath(full_path);

  for(int i=0; i<MAX_NUM_HOST_CONTACTS; i++) {
    if(clientContacts[i].host != NULL) {

      if(dumpHostToDB(clientContacts[i].host,
		      (family_id = HOST_FAMILY_ID) ?
		      ntop->getPrefs()->get_dump_hosts_to_db_policy() :
		      ntop->getPrefs()->get_dump_aggregations_to_db())) {
	char *host_ip;
	
	host_ip = clientContacts[i].host->print(buf, sizeof(buf));
	ntop->getRedis()->queueContactToDump(full_path, true, host_ip, family_id, clientContacts[i].num_contacts);
	
	if(family_id != HOST_FAMILY_ID) {
	  snprintf(alt_full_path, sizeof(alt_full_path), "%s/%c/%c|%s", path, host_ip[0], ifdot(host_ip[1]), host_ip);
	  ntop->fixPath(alt_full_path);
	  ntop->getRedis()->queueContactToDump(alt_full_path, true, key, family_id, clientContacts[i].num_contacts);
	}
      }
    }

    if(serverContacts[i].host != NULL) {
      if(dumpHostToDB(serverContacts[i].host,
		      (family_id = HOST_FAMILY_ID) ?
		      ntop->getPrefs()->get_dump_hosts_to_db_policy() :
		      ntop->getPrefs()->get_dump_aggregations_to_db())) {
	char *host_ip;

	host_ip = serverContacts[i].host->print(buf, sizeof(buf));
	ntop->getRedis()->queueContactToDump(full_path, false, host_ip, family_id, serverContacts[i].num_contacts);
	
	if(family_id != HOST_FAMILY_ID) {
	  snprintf(alt_full_path, sizeof(alt_full_path), "%s/%c/%c|%s", path, host_ip[0], ifdot(host_ip[1]), host_ip);
	  ntop->fixPath(alt_full_path);
	  ntop->getRedis()->queueContactToDump(alt_full_path, true, key, family_id, clientContacts[i].num_contacts);
	}
      } /* if */
    } /* if */
  } /* for */
}

