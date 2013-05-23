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
#include <pwd.h>

#ifdef DARWIN
#include <uuid/uuid.h>
#endif

/* **************************************************** */

PcapNetworkInterface::PcapNetworkInterface(char *name, bool change_user) : NetworkInterface(name, change_user) {
  char pcap_error_buffer[PCAP_ERRBUF_SIZE];

  if((pcap_handle = pcap_open_live(ifname, ntop->getGlobals()->getSnaplen(),
				   ntop->getGlobals()->getPromiscuousMode(),
				   500, pcap_error_buffer)) == NULL) {
    pcap_handle = pcap_open_offline(ifname, pcap_error_buffer);

    if(pcap_handle == NULL) {
      printf("ERROR: could not open pcap file: %s\n", pcap_error_buffer);
      exit(0);
    } else
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "Reading packets from pcap file %s...", ifname);
  } else
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Reading packets from interface %s...", ifname);  

  pcap_datalink_type = pcap_datalink(pcap_handle);

  if(change_user) dropPrivileges();
}

/* **************************************************** */

PcapNetworkInterface::~PcapNetworkInterface() {
  if(polling_started) {
    void *res;

    if(pcap_handle) pcap_breakloop(pcap_handle);
    pthread_join(pollLoop, &res);
  }

  if(pcap_handle)
    pcap_close(pcap_handle);

  deleteDataStructures();
}

/* **************************************************** */

static void pcap_packet_callback(u_char *args, const struct pcap_pkthdr *h, const u_char *packet) {
  NetworkInterface *iface = (NetworkInterface *) args;
  iface->packet_dissector(h, packet);
}

/* **************************************************** */

static void* packetPollLoop(void* ptr) {
  PcapNetworkInterface *iface = (PcapNetworkInterface*)ptr;

  pcap_loop(iface->get_pcap_handle(), -1, &pcap_packet_callback, (u_char*)iface);
  return(NULL);
}

/* **************************************************** */

void PcapNetworkInterface::startPacketPolling() {
  pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);
  NetworkInterface::startPacketPolling();
}

/* **************************************************** */

void PcapNetworkInterface::shutdown() {
  pcap_breakloop(pcap_handle);
}

/* **************************************************** */

