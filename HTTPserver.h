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

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include "ntop.h"

class HTTPserver {
 private:
  u_int16_t port;
  char *docs_dir, *scripts_dir;
  struct MHD_Daemon *httpd;


 public:
  HTTPserver(u_int16_t _port, const char *_docs_dir, const char *_scripts_dir);
  ~HTTPserver();

  inline char* get_docs_dir()    { return(docs_dir);    };
  inline char* get_scripts_dir() { return(scripts_dir); };
};


#endif /* _HTTP_SERVER_H_ */
