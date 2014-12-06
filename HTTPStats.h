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

#ifndef _HTTP_STATS_H_
#define _HTTP_STATS_H_

#include "ntop_includes.h"

struct http_request_stats {
  u_int32_t num_get, num_post, num_head, num_put, num_other;
};

struct http_responses_stats {
  u_int32_t num_100x, num_200x, num_300x, num_400x, num_500x;
};

class HTTPStats {
 private:
  struct http_request_stats req;
  struct http_responses_stats rsp;

 public:
  HTTPStats();

  void incRequest(char *method);
  void incResponse(char *return_code);

  char* serialize();
  void deserialize(json_object *o);
  json_object* getJSONObject();
  void lua(lua_State *vm);
};

#endif /* _HTTP_STATS_H_ */
