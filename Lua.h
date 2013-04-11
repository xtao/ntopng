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

#ifndef _LUA_H_
#define _LUA_H_

#include "ntop_includes.h"

/* ******************************* */

class Lua {
 private:
  lua_State *L;

 public:
  Lua();
  ~Lua();

  int handle_script_request(char *script_path,
			    void *cls,
			    struct MHD_Connection *connection,
			    const char *url,
			    const char *method,
			    const char *version,
			    const char *upload_data,
			    size_t *upload_data_size, void **ptr);
};

extern void lua_push_str_table_entry(lua_State *L, const char *key, char *value);
extern void lua_push_int_table_entry(lua_State *L, const char *key, u_int32_t value);

#endif /* _LUA_H_ */
