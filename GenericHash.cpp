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
 * This program is distributed in the ho2pe that it will be useful,
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

GenericHash::GenericHash(u_int _num_hashes, u_int _max_hash_size) {
  num_hashes = _num_hashes, max_hash_size = _max_hash_size, current_size = 0;

  table = new HashEntry*[num_hashes];
  for(u_int i = 0; i < num_hashes; i++)
    table[i] = NULL;

  locks = new Mutex*[num_hashes];
  for(u_int i = 0; i < num_hashes; i++) locks[i] = new Mutex();
}

/* ************************************ */

GenericHash::~GenericHash() {
  for(u_int i = 0; i < num_hashes; i++)
    if(table[i] != NULL) {
      HashEntry *head = table[i];

      while(head) {
	HashEntry *next = head->next();

	delete(head);
	head = next;
      }
    }

  delete[] table;

  for(u_int i = 0; i < num_hashes; i++) delete(locks[i]);
  delete[] locks;
}

/* ************************************ */

bool GenericHash::add(HashEntry *h) {
  if(current_size < max_hash_size) {
    u_int32_t hash = (h->key() % num_hashes);

    locks[hash]->lock(__FILE__, __LINE__);
    h->set_next(table[hash]);
    table[hash] = h, current_size++;
    locks[hash]->unlock(__FILE__, __LINE__);

    return(true);
  } else
    return(false);
}

/* ************************************ */

bool GenericHash::remove(HashEntry *h, bool lock_hash) {
  u_int32_t hash = (h->key() % num_hashes);

  if(table[hash] == NULL)
    return(false);
  else {
    HashEntry *head, *prev = NULL;
    bool ret;
    
    if(lock_hash) locks[hash]->lock(__FILE__, __LINE__);

    head = table[hash];
    while(head && (!head->equal(h))) {
      prev = head;
      head = head->next();
    }

    if(head) {
      if(prev != NULL)
	prev->set_next(head->next());
      else
	table[hash] = head->next();

      current_size--;

      ret = true;
    } else
      ret = false;
    
    if(lock_hash) locks[hash]->unlock(__FILE__, __LINE__);
    return(ret);
  }
}

/* ************************************ */

void GenericHash::walk(void (*walker)(HashEntry *h, void *user_data), void *user_data) {
  if(ntop->getGlobals()->isShutdown()) return;

  for(u_int i = 0; i < num_hashes; i++) {
    if(table[i] != NULL) {
      HashEntry *head;
      
      locks[i]->lock(__FILE__, __LINE__);
      head = table[i];

      while(head) {
	HashEntry *next = head->next();

	walker(head, user_data);
	head = next;
      } /* while */

      locks[i]->unlock(__FILE__, __LINE__);
    }
  }
}
