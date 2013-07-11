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

#ifndef _STRING_HOST_H_
#define _STRING_HOST_H_

#include "ntop_includes.h"

class StringHost : public GenericHost {
 private:
  char *keyname;

 public:
  StringHost(NetworkInterface *_iface, char *_key);
  ~StringHost();

  inline char* host_key() { return(keyname); };
  bool idle();
  inline u_int32_t key()  { return(Utils::hashString(keyname)); };
};

#endif /* _STRING_HOST_H_ */
