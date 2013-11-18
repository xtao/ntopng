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
#ifdef HAVE_SQLITE

/* ******************************************* */

DB::DB(NetworkInterface *_iface, u_int32_t _dir_duration) {
  dir_duration = max_val(_dir_duration, 300); /* 5 min is the minimum duration */

  // sqlite3_config(SQLITE_CONFIG_SERIALIZED);			 
  db = NULL, end_dump = 0, iface = _iface;
}

/* ******************************************* */

DB::~DB() {
  if(ntop->getPrefs()->do_dump_flows_on_db()) {
    void *res;
    
    pthread_join(dumpContactsThreadLoop, &res);
  }

  termDB();
}

/* **************************************************** */

static void* dumpContactsLoop(void* ptr) {
  DB *a = (DB*)ptr;
  Redis *r = ntop->getRedis();

  while(!ntop->getGlobals()->isShutdown()) {
    char todo[512];
    int rc = r->popContactToDump(todo, sizeof(todo));

    if(rc == 0) {
      a->execContactsSQLStatement(todo);
    } else
      sleep(1);
  }

  return(NULL);
}

/* **************************************************** */

void DB::startDumpContactsLoop() {
  pthread_create(&dumpContactsThreadLoop, NULL, dumpContactsLoop, (void*)this);
}

/* ******************************************* */

void DB::termDB() {
  if(!ntop->getPrefs()->do_dump_flows_on_db()) return;

  if(db) { 
    execSQL((char*)"COMMIT;");
    sqlite3_close(db);
    db = NULL; 
  }

  end_dump = 0;
}

/* ******************************************* */

void DB::initDB(time_t when, const char *create_sql_string) {
  char path[MAX_PATH];

  if(!ntop->getPrefs()->do_dump_flows_on_db()) return;

  if(db != NULL) {
    if(when < end_dump)
      return;
    else 
      termDB(); /* Too old: we first close it */
  }

  when -= when % dir_duration;

  strftime(path, sizeof(path), "%y/%m/%d/%H", localtime(&when));
  snprintf(db_path, sizeof(db_path), "%s/%s/flows/%s", 
	   ntop->get_working_dir(), iface->get_name(), path);
  ntop->fixPath(db_path);

  if(Utils::mkdir_tree(db_path)) {
    strftime(path, sizeof(path), "%y/%m/%d/%H/%M", localtime(&when));
    snprintf(db_path, sizeof(db_path), "%s/%s/flows/%s.sqlite",
	     ntop->get_working_dir(), iface->get_name(), path);

    end_dump = when + dir_duration;
    if(sqlite3_open(db_path, &db) != 0) {
      ntop->getTrace()->traceEvent(TRACE_ERROR,
				   "[DB] Unable to open/create DB %s [%s]",
				   db_path, sqlite3_errmsg(db));
      end_dump = 0, db = NULL;
    } else {
      execSQL((char*)create_sql_string);
      ntop->getTrace()->traceEvent(TRACE_INFO,
				   "[DB] Created %s", db_path);
    }
  } else
    ntop->getTrace()->traceEvent(TRACE_ERROR, 
				 "[DB] Unable to create directory tree %s", db_path);
}

/* ******************************************* */

bool DB::dumpFlow(time_t when, Flow *f) {
  const char *create_flows_db = "BEGIN; CREATE TABLE IF NOT EXISTS flows (vlan_id number, cli_ip string KEY, cli_port number, "
    "srv_ip string KEY, srv_port number, proto number, bytes number, duration number, json string);";
  char sql[4096], cli_str[64], srv_str[64], *json;
    
  initDB(when, create_flows_db);
    
  json = f->serialize();
  snprintf(sql, sizeof(sql), 
	   "INSERT INTO flows VALUES (%u, '%s', %u, '%s', %u, %lu, %u, %u, '%s');",
	   f->get_vlan_id(), 
	   f->get_cli_host()->get_ip()->print(cli_str, sizeof(cli_str)), 
	   f->get_cli_port(),
	   f->get_srv_host()->get_ip()->print(srv_str, sizeof(srv_str)), 
	   f->get_srv_port(),
	   (unsigned long)f->get_bytes(), f->get_duration(),
	   f->get_protocol(), json ? json : "");	   
    
  if(json) free(json);
  execSQL(sql);
  return(true);
}

/* ******************************************* */

bool DB::execSQL(char* sql) {
  if(ntop->getPrefs()->do_dump_flows_on_db()) {
    int rc;
    char *zErrMsg = 0;

    if(db == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] NULL DB handler [%s]", sql);
      return(false);
    }

    ntop->getTrace()->traceEvent(TRACE_INFO, "[DB] %s", sql);

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if(rc != SQLITE_OK) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] SQL error: %s [%s]", sql, zErrMsg);
      sqlite3_free(zErrMsg);    
      return(false);
    } else
      return(true);
  } else
    return(false);
}

/* ******************************************* */

bool DB::execContactsSQLStatement(char* _sql) {
  if(ntop->getPrefs()->do_dump_flows_on_db()) {
    char *zErrMsg = 0, *path, *filename, *sql, *where;
    sqlite3 *contacts_db;

    /* PATH|key|SQL */
    ntop->getTrace()->traceEvent(TRACE_INFO, "[DB] %s", _sql);

    if((path = strtok_r(_sql, "|", &where)) == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] Invalid SQL statement [%s]", _sql);
      return(false);
    }

    if((filename = strtok_r(NULL, "|", &where)) == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] Invalid SQL statement [%s]", _sql);
      return(false);
    }

    if((sql = strtok_r(NULL, "|", &where)) == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] Invalid SQL statement [%s]", _sql);
      return(false);
    }

    if(Utils::mkdir_tree(path)) {
      char db_path[MAX_PATH];
      struct stat buf;
      int rc;

      snprintf(db_path, sizeof(db_path), "%s/%s.sqlite", path, filename);
      ntop->fixPath(db_path);

      rc = stat(db_path, &buf);

      if(sqlite3_open(db_path, &contacts_db) != 0) {
	ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] Unable to open/create DB %s [%s]",
				     db_path, sqlite3_errmsg(db));
	return(false);
      }

      if(rc != 0) {
	const char *create_flows_db = "CREATE TABLE IF NOT EXISTS client_contacts (key string, family number, contacts number, PRIMARY KEY (key, family));"
	  "CREATE TABLE IF NOT EXISTS server_contacts (key string, family number, contacts number, PRIMARY KEY (key, family));";
	
	ntop->getTrace()->traceEvent(TRACE_INFO, "%s", create_flows_db);

	/* DB did not exist */
	rc = sqlite3_exec(contacts_db, create_flows_db, NULL, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
	  ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] SQL error: %s [%s]", sql, zErrMsg);
	  sqlite3_free(zErrMsg);   
	  sqlite3_close(contacts_db);
	  return(false);
	}
      }

      rc = sqlite3_exec(contacts_db, sql, NULL, 0, &zErrMsg);
      if(rc != SQLITE_OK) {
	ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] SQL error: %s [%s]", sql, zErrMsg);
	sqlite3_free(zErrMsg);   
	sqlite3_close(contacts_db);
      } else {
	sqlite3_close(contacts_db);
	return(true);
      }
    } else
      ntop->getTrace()->traceEvent(TRACE_ERROR, "[DB] Unable to create dir %s", path);
  }
  
  return(false);
}

/* ******************************************* */

#endif
