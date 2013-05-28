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

#ifdef HAVE_PF_RING

#include <pwd.h>

#ifdef DARWIN
#include <uuid/uuid.h>
#endif

/* **************************************************** */

PF_RINGInterface::PF_RINGInterface(const char *name, bool change_user)
  : NetworkInterface(name, change_user) {

  if((pfring_handle = pfring_open(ifname, ntop->getGlobals()->getSnaplen(),
				  ntop->getGlobals()->getPromiscuousMode() ? PF_RING_PROMISC : 0)) == NULL) {
    throw 1;
  } else
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Reading packets from interface %s...", ifname);

  pcap_datalink_type = DLT_EN10MB;

  if(change_user) dropPrivileges();
}

/* **************************************************** */

PF_RINGInterface::~PF_RINGInterface() {
  if(polling_started) {
    void *res;

    if(pfring_handle) pfring_breakloop(pfring_handle);
    pthread_join(pollLoop, &res);
  }

  if(pfring_handle)
    pfring_close(pfring_handle);

  deleteDataStructures();
}

/* **************************************************** */

static void pfring_packet_callback(const struct pfring_pkthdr *h, const u_char *p, const u_char *user_bytes) {
  NetworkInterface *iface = (NetworkInterface *) user_bytes;
  iface->packet_dissector((const struct pcap_pkthdr *) h, p);
}

/* **************************************************** */

static void* packetPollLoop(void* ptr) {
  PF_RINGInterface *iface = (PF_RINGInterface*)ptr;

  pfring_loop(iface->get_pfring_handle(), pfring_packet_callback, (u_char*) iface, 1 /* wait mode */);

  return(NULL);
}

/* **************************************************** */

void PF_RINGInterface::startPacketPolling() {
  pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);

  NetworkInterface::startPacketPolling();
}

/* **************************************************** */

void PF_RINGInterface::shutdown() {
  pfring_breakloop(pfring_handle);
}

/* **************************************************** */

u_int PF_RINGInterface::getNumDroppedPackets() {
  pfring_stat stats;
 
  if(pfring_stats(pfring_handle, &stats) >= 0)
    return(stats.drop);

  return(0);
}

/* **************************************************** */

#endif /* HAVE_PF_RING */
