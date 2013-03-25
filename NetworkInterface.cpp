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

#include "ntop.h"

/* **************************************************** */

NetworkInterface::NetworkInterface(NtopGlobals *globals, char *name) {
  char pcap_error_buffer[PCAP_ERRBUF_SIZE];

  ifname = strdup(name), ntopGlobals = globals;
  ifStats = new InterfaceStats(globals);
 
  if((pcap_handle = pcap_open_live(ifname, ntopGlobals->getSnaplen(),
				   ntopGlobals->getPromiscuousMode(),
				   500, pcap_error_buffer)) == NULL) {
    pcap_handle = pcap_open_offline(ifname, pcap_error_buffer);

    if(pcap_handle == NULL) {
      printf("ERROR: could not open pcap file: %s\n", pcap_error_buffer);
      throw "Unable to open network interface ";
    } else
      globals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "Reading packets from pcap file %s...", ifname);
  } else
    globals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "Reading packets from interface %s...", ifname);

  pcap_datalink_type = pcap_datalink(pcap_handle);
}

/* **************************************************** */

NetworkInterface::~NetworkInterface() {
  if(pcap_handle)
    pcap_close(pcap_handle);
  
  free(ifname);
  delete ifStats;
}

/* **************************************************** */

static void pcap_packet_callback(u_char * args, const struct pcap_pkthdr *header, const u_char * packet) {
  NetworkInterface *iface = (NetworkInterface*)args;

  iface->incStats(header->caplen);

#if 0
  const struct ndpi_ethhdr *ethernet;
  struct ndpi_iphdr *iph;
  u_int64_t time;
  static u_int64_t lasttime = 0;
  u_int16_t type, ip_offset;

  time = ((uint64_t) header->ts.tv_sec) * detection_tick_resolution + header->ts.tv_usec / (1000000 / detection_tick_resolution);
  if (lasttime > time) time = lasttime;
  
  lasttime = time;

  if(_pcap_datalink_type == DLT_EN10MB) {
    ethernet = (struct ndpi_ethhdr *) packet;
    ip_offset = sizeof(struct ndpi_ethhdr);
    type = ntohs(ethernet->h_proto);
  } else if(_pcap_datalink_type == 113 /* Linux Cooked Capture */) {
    type = packet[14] << 8 + packet[15];
    ip_offset = 16;
  } else
    return;

  if(type == 0x8100 /* VLAN */) {
    type = packet[ip_offset+2] << 8 + packet[ip_offset+3];
    ip_offset += 4;
  }

  iph = (struct ndpi_iphdr *) &packet[ip_offset];

  // just work on Ethernet packets that contain IP
  if (type == ETH_P_IP && header->caplen >= ip_offset) {
    u_int16_t frag_off = ntohs(iph->frag_off);

    if(header->caplen < header->len) {
      static u_int8_t cap_warning_used = 0;
      if (cap_warning_used == 0) {
	printf("\n\nWARNING: packet capture size is smaller than packet size, DETECTION MIGHT NOT WORK CORRECTLY\n\n");
	cap_warning_used = 1;
      }
    }

    if (iph->version != 4) {
      static u_int8_t ipv4_warning_used = 0;

    v4_warning:
      if (ipv4_warning_used == 0) {
	printf("\n\nWARNING: only IPv4 packets are supported in this demo (nDPI supports both IPv4 and IPv6), all other packets will be discarded\n\n");
	ipv4_warning_used = 1;
      }
      return;
    }

    if(decode_tunnels && (iph->protocol == IPPROTO_UDP) && ((frag_off & 0x3FFF) == 0)) {
      u_short ip_len = ((u_short)iph->ihl * 4);
      struct ndpi_udphdr *udp = (struct ndpi_udphdr *)&packet[ip_offset+ip_len];
      u_int16_t sport = ntohs(udp->source), dport = ntohs(udp->dest);

      if((sport == GTP_U_V1_PORT) || (dport == GTP_U_V1_PORT)) {
	/* Check if it's GTPv1 */
	u_int offset = ip_offset+ip_len+sizeof(struct ndpi_udphdr);
	u_int8_t flags = packet[offset];
	u_int8_t message_type = packet[offset+1];

	if((((flags & 0xE0) >> 5) == 1 /* GTPv1 */) && (message_type == 0xFF /* T-PDU */)) {
	  ip_offset = ip_offset+ip_len+sizeof(struct ndpi_udphdr)+8 /* GTPv1 header len */;

	  if(flags & 0x04) ip_offset += 1; /* next_ext_header is present */
	  if(flags & 0x02) ip_offset += 4; /* sequence_number is present (it also includes next_ext_header and pdu_number) */
	  if(flags & 0x01) ip_offset += 1; /* pdu_number is present */

	  iph = (struct ndpi_iphdr *) &packet[ip_offset];

	  if (iph->version != 4) {
	    // printf("WARNING: not good (packet_id=%u)!\n", (unsigned int)raw_packet_count);
	    goto v4_warning;
	  }
	}
      }

    }

    // process the packet
    packet_processing(time, iph, header->len - ip_offset, header->len);
  }
#endif
}

/* **************************************************** */

void NetworkInterface::startPacketPolling() {
  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "Started packet polling...");

  pcap_loop(pcap_handle, -1, &pcap_packet_callback, (u_char*)this);
}

/* **************************************************** */

void NetworkInterface::shutdown() {
  pcap_breakloop(pcap_handle);
}

/* **************************************************** */

