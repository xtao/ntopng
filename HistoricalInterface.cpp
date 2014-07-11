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

#include "ntop_includes.h"

/* **************************************************** */

HistoricalInterface::HistoricalInterface(u_int8_t _id, const char *_endpoint)
  : ParserInterface(_id, _endpoint) {

    num_historicals = 0, num_query_error = 0, num_open_error = 0, num_missing_file = 0;
    from_epoch = 0, to_epoch = 0, interface_id = 0;
    have_endpoint = false;

    setEndpoint(_endpoint);

}

/* **************************************************** */

HistoricalInterface::~HistoricalInterface() {
  // Free the instance variables
  for(int i=0; i<num_historicals; i++) {
    if(historical_ifaces[i].endpoint) free(historical_ifaces[i].endpoint);
   sqlite3_close(historical_ifaces[num_historicals].db);
  }
}


/* **************************************************** */

void HistoricalInterface::cleanUp() {
  // Free the instance variables
  for(int i=0; i<num_historicals; i++) {
    if(historical_ifaces[i].endpoint) free(historical_ifaces[i].endpoint);
   sqlite3_close(historical_ifaces[num_historicals].db);
  }
  num_historicals = 0, num_query_error = 0, num_open_error = 0, num_missing_file = 0;
  from_epoch = 0, to_epoch = 0, interface_id = 0;
  have_endpoint = false;
  sprobe_interface = false;

}


/* **************************************************** */

void HistoricalInterface::parse_endpoint(const  char * p_endpoint){
    char *tmp, *e;
    time_t actual_epoch;
    char path[MAX_PATH];
    char db_path[MAX_PATH];
    struct stat buf;

   if((tmp = strdup(p_endpoint)) == NULL) throw("Out of memory");

    if(strstr(tmp, "sqlite")) {
      // Endpoint defined via command line
      e = strtok(tmp, ",");
      while(e != NULL) {
      if(num_historicals == CONST_MAX_NUM_SQLITE_INTERFACE) {
        ntop->getTrace()->traceEvent(TRACE_ERROR,
           "Too many historical file defined %u: skipping those in excess",
           num_historicals);
      break;
      }

      if(stat(e, &buf) != 0){
          ntop->getTrace()->traceEvent(TRACE_NORMAL,"Missing file: %s",e);
          num_missing_file++;
          continue;
      }

      historical_ifaces[num_historicals].endpoint = strdup(e);
      num_historicals++;
      ntop->getTrace()->traceEvent(TRACE_NORMAL,"Add historical file from cli: %s",e);

      e = strtok(NULL, ",");

      }

      if(num_historicals > 0)
        have_endpoint = true;

    } else  if(strstr(tmp, ",")) {
      // Endpoint defined via GUI

      e = strtok(tmp, ",");
      this->from_epoch = atoi(e);

      e = strtok(NULL, ",");
      this->to_epoch = atoi(e);

      e = strtok(NULL, ",");
      this->interface_id = atoi(e);

      actual_epoch = from_epoch;
      while (actual_epoch <= to_epoch){

        if(num_historicals == CONST_MAX_NUM_SQLITE_INTERFACE) {
          ntop->getTrace()->traceEvent(TRACE_ERROR,
            "Too many historical file defined %u: skipping those in excess",
            num_historicals);
          break;
        }
        memset(path, 0, sizeof(path));
        memset(db_path, 0, sizeof(db_path));

        strftime(path, sizeof(path), "%Y/%m/%d/%H/%M", localtime(&actual_epoch));
        snprintf(db_path, sizeof(db_path), "%s/%u/flows/%s.sqlite",
        ntop->get_working_dir(), interface_id , path);

        if(stat(db_path, &buf) != 0){
          ntop->getTrace()->traceEvent(TRACE_NORMAL,"Missing file: %s",db_path);
          num_missing_file++;
          goto new_epoch;
        }

        historical_ifaces[num_historicals].endpoint = strdup(db_path);
        num_historicals++;
        ntop->getTrace()->traceEvent(TRACE_NORMAL,"Add historical file from gui: %s",db_path);

        new_epoch:
          actual_epoch += 300; //5 minutes
      }

      if(num_historicals > 0)
        have_endpoint = true;

    }

    free(tmp);

}

/* **************************************************** */

int HistoricalInterface::sqlite_callback(void *data, int argc,
         char **argv, char **azColName) {

  for(int i=0; i<argc; i++) {
    // Inject only the json information
    if ( (strcmp( (const char*)azColName[i], "json") == 0 ) &&
         (char*)(argv[i]) ) {

        parse_flows( (char*)(argv[i]) , sizeof((char*)(argv[i])) , 0, data);
        // ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s = %s", (const char*)azColName[i],(char*)(argv[i]));
        // ntop->getTrace()->traceEvent(TRACE_NORMAL, "sizeof %d", sizeof((char*)(argv[i])) );
    }

  }

  return(0);
}

/* **************************************************** */

void HistoricalInterface::collect_flows() {

  char *zErrMsg = 0;

   for(int i=0; i<num_historicals; i++) {

    if(sqlite3_open(historical_ifaces[i].endpoint, &historical_ifaces[i].db)) {
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "Unable to open %s: %s",
      historical_ifaces[i].endpoint, sqlite3_errmsg(historical_ifaces[i].db));
      num_open_error++;
    } else
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "Open db %s", historical_ifaces[i].endpoint);

      // Correctly open db, so now we can extract the contained flows
    if(sqlite3_exec(historical_ifaces[i].db, "SELECT * FROM flows ORDER BY first_seen, srv_ip, srv_port, cli_ip, cli_port ASC", sqlite_callback, this, &zErrMsg)) {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "SQL Error: %s from %s", zErrMsg, historical_ifaces[i].endpoint);
      sqlite3_free(zErrMsg);
      num_query_error++;
      goto close_db;
    }

     ntop->getTrace()->traceEvent(TRACE_NORMAL, "Terminated flow polling from %s", historical_ifaces[i].endpoint );

     close_db:
      sqlite3_close(historical_ifaces[i].db);
   }

}

/* **************************************************** */

static void* packetPollLoop(void* ptr) {
  HistoricalInterface *iface = (HistoricalInterface*)ptr;

  /* Wait until the initialization completes */
  while(!iface->isRunning()) sleep(1);

  iface->collect_flows();
  return(NULL);
}

/* **************************************************** */

void HistoricalInterface::startPacketPolling() {
  if (have_endpoint) {
    pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);
    NetworkInterface::startPacketPolling();
  }
}

/* **************************************************** */

void HistoricalInterface::shutdown() {
  void *res;

  if(running) {
    NetworkInterface::shutdown();
    pthread_join(pollLoop, &res);
  }
}

/* **************************************************** */

bool HistoricalInterface::set_packet_filter(char *filter) {
  ntop->getTrace()->traceEvent(TRACE_ERROR,
             "No filter can be set on a historical interface. Ignored %s", filter);
  return(false);
}

/* **************************************************** */

void HistoricalInterface::setEndpoint(const char * p_endpoint) {
    if (p_endpoint != NULL) {
      parse_endpoint(p_endpoint);
    } else {
      running = false;
    }
}