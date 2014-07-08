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

    char *tmp, *e;

    num_historicals = 0;

    if((tmp = strdup(_endpoint)) == NULL) throw("Out of memory");

    e = strtok(tmp, ",");
    while(e != NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR,
           "Too many historical interface defined %u: skipping those in excess",
           num_historicals);
      if(num_historicals == CONST_MAX_NUM_SQLITE_INTERFACE) {
        ntop->getTrace()->traceEvent(TRACE_ERROR,
           "Too many historical interface defined %u: skipping those in excess",
           num_historicals);
      break;
    }

      historical_ifaces[num_historicals].endpoint = strdup(e);

      if(sqlite3_open(historical_ifaces[num_historicals].endpoint, &historical_ifaces[num_historicals].db)) {
        ntop->getTrace()->traceEvent(TRACE_INFO, "Unable to open %s: %s",
        historical_ifaces[num_historicals].endpoint, sqlite3_errmsg(historical_ifaces[num_historicals].db));
      } else
        ntop->getTrace()->traceEvent(TRACE_INFO, "Open db %s", historical_ifaces[num_historicals].endpoint);

      num_historicals++;
      e = strtok(NULL, ",");

  }

  free(tmp);

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

    if(sqlite3_exec(historical_ifaces[i].db, "SELECT * FROM flows ORDER BY first_seen, srv_ip, srv_port, cli_ip, cli_port ASC", sqlite_callback, this, &zErrMsg)) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "SQL Error: %s", zErrMsg);
      sqlite3_free(zErrMsg);
    }

    sqlite3_close(historical_ifaces[i].db);
     ntop->getTrace()->traceEvent(TRACE_NORMAL, "Terminated flow polling for %s", historical_ifaces[i].endpoint );
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
  pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);
  NetworkInterface::startPacketPolling();
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
