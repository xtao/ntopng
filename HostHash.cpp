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

/* ************************************ */

HostHash::HostHash(u_int hsize) {
  max_hash_size = hsize, current_size = 0;

  table = new Host*[max_hash_size];
  for(u_int i = 0; i < max_hash_size; i++)
    table[i] = NULL;
}
 
/* ************************************ */

HostHash::~HostHash() {
  for(u_int i = 0; i < max_hash_size; i++)
    if(table[i] != NULL) {
      Host *head = table[i];

      while(head) {
	Host *next = head->next();

	delete(head);
	head = next;
      }
    }

  delete[] table;
}

/* ************************************ */

Host* HostHash::get(IpAddress *key) {
  u_int32_t hash = (key->key() % max_hash_size);

  if(table[hash] == NULL)
    return(NULL);
  else {
    Host *head = table[hash];
    
    while(head && (head->get_ip()->compare(key) != 0))
      head = head->next();
    
    return(head);
  }
}

/* ************************************ */
 
bool HostHash::add(Host *h) {
  if(current_size < max_hash_size) {
    u_int32_t hash = (h->key() % max_hash_size);
    
    h->set_next(table[hash]);
    table[hash] = h, current_size++;

    return(true);
  } else
    return(false);
}     

/* ************************************ */
 
bool HostHash::remove(Host *h) {
  u_int32_t hash = (h->key() % max_hash_size);
  
  if(table[hash] == NULL)
    return(false);
  else {
    Host *head = table[hash], *prev = NULL;

    while(head && (head->get_ip()->compare(h->get_ip()) != 0)) {
      prev = head;
      head = head->next();
    }

    if(head) {
      if(prev != NULL)
	prev->set_next(head->next());
      else
	table[hash] = head->next();

      return(true);
    } else
      return(false);
  }
}     

/* ************************************ */

void HostHash::walk(void (*walker)(Host *h, void *user_data), void *user_data) {
  for(u_int i = 0; i < max_hash_size; i++)
    if(table[i] != NULL) {
      Host *head = table[i];

      while(head) {
	Host *next = head->next();

	walker(head, user_data);
	head = next;
      }
    }
}
