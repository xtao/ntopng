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

#ifndef _DB_CLASS_H_
#define _DB_CLASS_H_

#include "ntop_includes.h"

#ifdef HAVE_SQLITE

class DB {
 private:
  sqlite3 *db, *contacts_db;
  NetworkInterface *iface;
  u_int32_t dir_duration, num_contacts_db_insert;
  char db_path[MAX_PATH], last_open_contacts_db_path[MAX_PATH];
  time_t end_dump;
  pthread_t dumpContactsThreadLoop;

  void initDB(time_t when, const char *create_sql_string);
  void termDB();
  bool execSQL(char* sql);  
  
 public:
  DB(NetworkInterface *_iface = NULL,
     u_int32_t _dir_duration = 300 /* 5 minutes */);
  ~DB();

  void startDumpContactsLoop();
  bool dumpFlow(time_t when, Flow *f);
  void dumpContacts(HostContacts *c, char *path);
  bool execContactsSQLStatement(char* _sql);
};

#endif /* HAVE_SQLITE */

#endif /* _DB_CLASS_H_ */
