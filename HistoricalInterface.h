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

#ifndef _HISTORICAL_INTERFACE_H_
#define _HISTORICAL_INTERFACE_H_

#include "ntop_includes.h"

 typedef struct {
  char *endpoint;
  sqlite3 *db;
} sqlite_iface;

class HistoricalInterface : public ParserInterface {
 private:
  u_int8_t num_historicals;
  sqlite_iface historical_ifaces[CONST_MAX_NUM_SQLITE_INTERFACE];

  static int sqlite_callback(void *data, int argc, char **argv, char **azColName);

 public:
  HistoricalInterface(u_int8_t _id, const char *_endpoint);
  ~HistoricalInterface();

  // Father function
  inline const char* get_type()         { return("sqlite");      };
  inline bool is_ndpi_enabled()         { return(false);      };
  // char* getEndpoint(u_int8_t id)        { return(endpoint); };

  char* getEndpoint(u_int8_t id)        { return((id < num_historicals) ?
             historical_ifaces[id].endpoint : (char*)""); };
  inline u_int getNumDroppedPackets()   { return 0; };

  bool set_packet_filter(char *filter);


  void collect_flows();
  void startPacketPolling();
  void shutdown();


};

#endif /* _HISTORICAL_INTERFACE_H_ */

