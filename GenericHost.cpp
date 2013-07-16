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

/* *************************************** */

GenericHost::GenericHost(NetworkInterface *_iface) : GenericHashEntry(_iface) {
  ndpiStats = new NdpiStats();
  
}

/* *************************************** */

GenericHost::~GenericHost() {
  if(ndpiStats)
    delete ndpiStats;
}

/* *************************************** */

void GenericHost::incStats(u_int8_t l4_proto, u_int ndpi_proto, 
			   u_int64_t sent_packets, u_int64_t sent_bytes,
			   u_int64_t rcvd_packets, u_int64_t rcvd_bytes) { 
  if(sent_packets || rcvd_packets) {
    sent.incStats(sent_packets, sent_bytes), rcvd.incStats(rcvd_packets, rcvd_bytes);

    if((ndpi_proto != NO_NDPI_PROTOCOL) && ndpiStats)
      ndpiStats->incStats(ndpi_proto, sent_packets, sent_bytes, rcvd_packets, rcvd_bytes);      
 
   updateSeen();
  }
}

/* *************************************** */

void GenericHost::incrContact(char *me, char *peer, 
			      bool contacted_peer_as_client) {
  char key[128];

  snprintf(key, sizeof(key), "%s.%s",
	   me, contacted_peer_as_client ? "client" : "server");
  
  ntop->getRedis()->zincrbyAndTrim(key, peer, 1 /* +1 */, MAX_NUM_HOST_CONTACTS);
  
#if 0
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s contacted %s as %s",
			       me, peer,
			       contacted_peer_as_client ? "client" : "server");
#endif

}
