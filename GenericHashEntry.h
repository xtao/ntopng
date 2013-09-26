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

#ifndef _GENERIC_HASH_ENTRY_H_
#define _GENERIC_HASH_ENTRY_H_

#include "ntop_includes.h"

class GenericHashEntry {
 private:
  GenericHashEntry *hash_next;

 protected:
  time_t first_seen, last_seen;
  NetworkInterface *iface;

 public:
  GenericHashEntry(NetworkInterface *_iface);
  virtual ~GenericHashEntry();

  inline time_t get_first_seen()     { return(first_seen); };
  inline time_t get_last_seen()      { return(last_seen); };
  inline GenericHashEntry* next()    { return(hash_next); };
  inline void set_next(GenericHashEntry *n) { hash_next = n;     };
  void updateSeen();
  void updateSeen(time_t _last_seen);
  bool equal(GenericHashEntry *b)    { return((this == b) ? true : false); };  
  inline NetworkInterface* getInterface() { return(iface); };
  virtual bool idle();
  inline u_int get_duration()        { return((u_int)(1+last_seen-first_seen)); };
  virtual u_int32_t key()            { return(0);         };  
  virtual char* get_string_key(char *buf, u_int buf_len) { buf[0] = '\0'; return(buf); };
  virtual bool isIdle(u_int max_idleness);
};

#endif /* _GENERIC_HASH_ENTRY_H_ */
