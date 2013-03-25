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

#ifndef _NETWORK_INTERFACE_H_
#define _NETWORK_INTERFACE_H_

#include "ntop.h"

class NetworkInterface {
 private:
  char *ifname;
  NtopGlobals *ntopGlobals;
  InterfaceStats *ifStats;
  pcap_t *pcap_handle;
  int pcap_datalink_type;

 public:
  NetworkInterface(NtopGlobals *globals, char *name);
  ~NetworkInterface();
  void startPacketPolling();
  void shutdown();
  inline void incStats(u_int pkt_len) { ifStats->incStats(pkt_len);      };  
  inline Trace* getTrace()            { return(ntopGlobals->getTrace()); };
  inline InterfaceStats* getStats()   { return(ifStats);                 };
};

#endif /* _NETWORK_INTERFACE_H_ */
