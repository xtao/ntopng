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

#ifndef _HOST_HASH_H_
#define _HOST_HASH_H_

#include "ntop_includes.h"
 
class HostHash {
 private:
  Host **table;
  u_int max_hash_size, current_size;

 public:
  HostHash(u_int hsize);
  ~HostHash();
 
  Host* get(IpAddress *key);
  bool add(Host *h);
  bool remove(Host *h); /* Note: Host* memory is NOT freed */
  void walk(void (*walker)(Host *h, void *user_data), void *user_data);
};

#endif /* _HOST_HASH_H_ */
