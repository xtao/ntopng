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

#ifndef _HOST_CONTACTS_H_
#define _HOST_CONTACTS_H_

#include "ntop_includes.h"

typedef struct {
  IpAddress *host;
  u_int32_t num_contacts;
} IPContacts;

class HostContacts : public Serializable {
 protected:
  IPContacts clientContacts[MAX_NUM_HOST_CONTACTS], serverContacts[MAX_NUM_HOST_CONTACTS];  
  void incrIPContacts(IpAddress *peer, IPContacts *contacts, u_int32_t value);

 public:
  HostContacts();
  ~HostContacts();

  inline void incrContact(IpAddress *peer, bool contacted_peer_as_client, u_int32_t value=1) { 
    incrIPContacts(peer, contacted_peer_as_client ? clientContacts : serverContacts, value);
  };

  u_int get_num_contacts_by(IpAddress* host_ip);

  void getIPContacts(lua_State* vm);
  char* serialize();
  void deserialize(json_object *o);
  json_object* getJSONObject();
};

#endif /* _HOST_CONTACTS_H_ */
