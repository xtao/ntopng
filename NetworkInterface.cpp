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

#ifdef __APPLE__
#include <uuid/uuid.h>
#endif

static bool help_printed = false;

/* **************************************** */

static void debug_printf(u_int32_t protocol, void *id_struct,
			 ndpi_log_level_t log_level,
			 const char *format, ...) {
}

/* **************************************** */

static void *malloc_wrapper(unsigned long size)
{
  return malloc(size);
}

/* **************************************** */

static void free_wrapper(void *freeable)
{
  free(freeable);
}

/* **************************************************** */

NetworkInterface::NetworkInterface() {
  ifname = NULL, flows_hash = NULL, hosts_hash = NULL,
    strings_hash = NULL, ndpi_struct = NULL,
    purge_idle_flows_hosts = true;
}

/* **************************************************** */

NetworkInterface::NetworkInterface(const char *name) {
  char pcap_error_buffer[PCAP_ERRBUF_SIZE];
  NDPI_PROTOCOL_BITMASK all;
  u_int32_t num_hashes;
  char _ifname[64];

#ifdef WIN32
  if(name == NULL) name = "1"; /* First available interface */
#endif

  if(name == NULL) {
    if(!help_printed) ntop->getTrace()->traceEvent(TRACE_WARNING, "No capture interface specified");
    printAvailableInterfaces(false, 0, NULL, 0);

    name = pcap_lookupdev(pcap_error_buffer);

    if(name == NULL) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate default interface (%s)\n", pcap_error_buffer);
      exit(0);
    }
  } else {
    if(isNumber(name)) {
      /* We need to convert this numeric index into an interface name */
      int id = atoi(name);

      _ifname[0] = '\0';
      printAvailableInterfaces(false, id, _ifname, sizeof(_ifname));

      if(_ifname[0] == '\0') {
	ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to locate interface Id %d", id);
	printAvailableInterfaces(false, 0, NULL, 0);
	exit(0);
      }
      name = _ifname;
    }
  }

  ifname = strdup(name);

  num_hashes = max_val(4096, ntop->getPrefs()->get_max_num_flows()/4);
  flows_hash = new FlowHash(this, num_hashes, ntop->getPrefs()->get_max_num_flows());

  num_hashes = max_val(4096, ntop->getPrefs()->get_max_num_hosts()/4);
  hosts_hash = new HostHash(this, num_hashes, ntop->getPrefs()->get_max_num_hosts());
  strings_hash = new StringHash(this, num_hashes, ntop->getPrefs()->get_max_num_hosts());

  // init global detection structure
  ndpi_struct = ndpi_init_detection_module(ntop->getGlobals()->get_detection_tick_resolution(),
					   malloc_wrapper, free_wrapper, debug_printf);
  if(ndpi_struct == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Global structure initialization failed");
    exit(-1);
  }

  if(ntop->getCustomnDPIProtos() != NULL)
    ndpi_load_protocols_file(ndpi_struct, ntop->getCustomnDPIProtos());

  // enable all protocols
  NDPI_BITMASK_SET_ALL(all);
  ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);

  last_pkt_rcvd = 0;
  next_idle_flow_purge = next_idle_host_purge = next_idle_aggregated_host_purge = 0;
  cpu_affinity = -1;
  running = false;
}

/* **************************************************** */

void NetworkInterface::deleteDataStructures() {
  if(flows_hash) { delete flows_hash; flows_hash = NULL; }
  if(hosts_hash) { delete hosts_hash; hosts_hash = NULL; }
  if(strings_hash) { delete strings_hash; strings_hash = NULL; }

  if(ndpi_struct) {
    ndpi_exit_detection_module(ndpi_struct, free_wrapper);
    ndpi_struct = NULL;
  }

  if(ifname) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Interface %s shutdown", ifname);
    free(ifname);
    ifname = NULL;
  }
}

/* **************************************************** */

NetworkInterface::~NetworkInterface() {
  deleteDataStructures();
}

/* **************************************************** */

static void node_proto_guess_walker(GenericHashEntry *node, void *user_data) {
  Flow *flow = (Flow*)node;

  flow->print();
}

/* **************************************************** */

void NetworkInterface::dumpFlows() {
  flows_hash->walk(node_proto_guess_walker, NULL);
}

/* **************************************************** */

Flow* NetworkInterface::getFlow(u_int8_t *src_eth, u_int8_t *dst_eth, u_int16_t vlan_id,
  				IpAddress *src_ip, IpAddress *dst_ip,
  				u_int16_t src_port, u_int16_t dst_port,
				u_int8_t l4_proto,
				bool *src2dst_direction,
				time_t first_seen, time_t last_seen) {
  Flow *ret;

  ret = flows_hash->find(src_ip, dst_ip, src_port, dst_port, vlan_id, l4_proto, src2dst_direction);

  if(ret == NULL) {
    ret = new Flow(this, vlan_id, l4_proto,
		   src_eth, src_ip, src_port,
		   dst_eth, dst_ip, dst_port,
		   first_seen, last_seen);
    if(flows_hash->add(ret)) {
      *src2dst_direction = true;
      return(ret);
    } else {
      delete ret;
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many flows");
      return(NULL);
    }
  } else
    return(ret);
}

/* **************************************************** */

void NetworkInterface::flow_processing(u_int8_t *src_eth, u_int8_t *dst_eth,
				       IpAddress *src_ip, IpAddress *dst_ip,
				       u_int16_t src_port, u_int16_t dst_port,
				       u_int16_t vlan_id,
				       u_int16_t proto_id,
				       u_int8_t l4_proto, u_int8_t tcp_flags,
				       u_int in_pkts, u_int in_bytes,
				       u_int out_pkts, u_int out_bytes,
				       u_int first_switched, u_int last_switched,
				       char *additional_fields_json)
{
  bool src2dst_direction;
  Flow *flow;

  if (last_switched > last_pkt_rcvd) last_pkt_rcvd = last_switched;

  /* Updating Flow */

  flow = getFlow(src_eth, dst_eth, vlan_id, src_ip, dst_ip, src_port, dst_port,
		 l4_proto, &src2dst_direction, first_switched, last_switched);

  if(flow == NULL) return;

  if(l4_proto == IPPROTO_TCP) flow->updateTcpFlags(tcp_flags);
  flow->addFlowStats(src2dst_direction, in_pkts, in_bytes, out_pkts, out_bytes, last_switched);
  flow->setDetectedProtocol(proto_id);
  flow->setJSONInfo(additional_fields_json);
  incStats(src_ip->isIPv4() ? ETHERTYPE_IP : ETHERTYPE_IPV6,
	   flow->get_detected_protocol(), in_bytes+out_bytes, (in_pkts+out_pkts),
	   24 /* 8 Preamble + 4 CRC + 12 IFG */ + 14 /* Ethernet header */);

  purgeIdle(last_switched);
}

/* **************************************************** */

void NetworkInterface::packet_processing(const u_int32_t when,
					 const u_int64_t time,
					 struct ndpi_ethhdr *eth,
					 u_int16_t vlan_id,
					 struct ndpi_iphdr *iph,
					 struct ndpi_ip6_hdr *ip6,
					 u_int16_t ipsize, u_int16_t rawsize)
{
  bool src2dst_direction;
  u_int8_t l4_proto;
  Flow *flow;
  u_int8_t *eth_src = eth->h_source, *eth_dst = eth->h_dest;
  IpAddress src_ip, dst_ip;
  u_int16_t src_port, dst_port;
  struct ndpi_tcphdr *tcph = NULL;
  struct ndpi_udphdr *udph = NULL;
  u_int16_t l4_packet_len;
  u_int8_t *l4, tcp_flags = 0;
  u_int8_t *ip;
  bool is_fragment = false;

  if(iph != NULL) {
    /* IPv4 */
    if(ipsize < 20) {
      incStats(ETHERTYPE_IP, NDPI_PROTOCOL_UNKNOWN, rawsize, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
      return;
    }

    if((iph->ihl * 4) > ipsize || ipsize < ntohs(iph->tot_len)
       || (iph->frag_off & htons(0x1FFF /* IP_OFFSET */)) != 0) {
      is_fragment = true;
    }

    l4_packet_len = ntohs(iph->tot_len) - (iph->ihl * 4);
    l4_proto = iph->protocol;
    l4 = ((u_int8_t *) iph + iph->ihl * 4);
    ip = (u_int8_t*)iph;
  } else {
    /* IPv6 */
    if(ipsize < sizeof(const struct ndpi_ip6_hdr)) {
      incStats(ETHERTYPE_IPV6, NDPI_PROTOCOL_UNKNOWN, rawsize, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
      return;
    }

    l4_packet_len = ntohs(ip6->ip6_ctlun.ip6_un1.ip6_un1_plen)-sizeof(const struct ndpi_ip6_hdr);
    l4_proto = ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    l4 = (u_int8_t*)ip6 + sizeof(const struct ndpi_ip6_hdr);
    ip = (u_int8_t*)ip6;
  }

  if((l4_proto == IPPROTO_TCP) && (l4_packet_len >= 20)) {
    /* tcp */
    tcph = (struct ndpi_tcphdr *)l4;
    src_port = tcph->source, dst_port = tcph->dest;
    tcp_flags = l4[13];
  } else if((l4_proto == IPPROTO_UDP) && (l4_packet_len >= 8)) {
    /* udp */
    udph = (struct ndpi_udphdr *)l4;
    src_port = udph->source,  dst_port = udph->dest;
  } else {
    /* non tcp/udp protocols */

    src_port = dst_port = 0;
  }

  if(iph != NULL) {
    src_ip.set_ipv4(iph->saddr);
    dst_ip.set_ipv4(iph->daddr);
  } else {
    src_ip.set_ipv6(&ip6->ip6_src);
    dst_ip.set_ipv6(&ip6->ip6_dst);
  }

#if defined(WIN32) && defined(DEMO_WIN32)
  if(this->ethStats.getNumPackets() > MAX_NUM_PACKETS) {
    static bool showMsg = false;

    if(!showMsg) {
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "-----------------------------------------------------------");
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "WARNING: this demo application is a limited ntopng version able to");
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "capture up to %d packets. If you are interested", MAX_NUM_PACKETS);
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "in the full version please have a look at the ntop");
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "home page http://www.ntop.org/.");
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "-----------------------------------------------------------");
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "");
      showMsg = true;
    }

    return;
  }
#endif

  /* Updating Flow */
  flow = getFlow(eth_src, eth_dst, vlan_id, &src_ip, &dst_ip, src_port, dst_port,
		 l4_proto, &src2dst_direction, last_pkt_rcvd, last_pkt_rcvd);

  if(flow == NULL) {
    incStats(iph ? ETHERTYPE_IP : ETHERTYPE_IPV6, NDPI_PROTOCOL_UNKNOWN, rawsize, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
    return;
  } else {
    flow->incStats(src2dst_direction, rawsize);
    if(l4_proto == IPPROTO_TCP) flow->updateTcpFlags(tcp_flags);
  }

  /* Protocol Detection */

  if(flow->isDetectionCompleted()) {
    flow->processDetectedProtocol();
    flow->deleteFlowMemory(); 
    incStats(iph ? ETHERTYPE_IP : ETHERTYPE_IPV6, flow->get_detected_protocol(), rawsize, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
    return;
  } else
    incStats(iph ? ETHERTYPE_IP : ETHERTYPE_IPV6, NDPI_PROTOCOL_UNKNOWN, rawsize, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);

  if(!is_fragment) {
    struct ndpi_flow_struct *ndpi_flow = flow->get_ndpi_flow();
    struct ndpi_id_struct *cli = (struct ndpi_id_struct*)flow->get_cli_id();
    struct ndpi_id_struct *srv = (struct ndpi_id_struct*)flow->get_srv_id();

    flow->setDetectedProtocol(ndpi_detection_process_packet(ndpi_struct, ndpi_flow,
							    ip, ipsize, (u_int32_t)time,
							    cli, srv));
  } else {
    // FIX - only handle unfragmented packets
    // ntop->getTrace()->traceEvent(TRACE_WARNING, "IP fragments are not handled yet!");
  }
}

/* **************************************************** */

void NetworkInterface::purgeIdle(time_t when) {
  u_int n;

  last_pkt_rcvd = when;

  if((n = purgeIdleFlows()) > 0)
    ntop->getTrace()->traceEvent(TRACE_INFO, "Purged %u/%u idle flows",
				 n, getNumFlows());

  if((n = purgeIdleHosts()) > 0)
    ntop->getTrace()->traceEvent(TRACE_INFO, "Purged %u/%u idle hosts",
				 n, getNumHosts());

  if((n = purgeIdleAggregatedHosts()) > 0)
    ntop->getTrace()->traceEvent(TRACE_INFO, "Purged %u/%u idle aggregated hosts",
				 n, getNumHosts());
}

/* **************************************************** */

void NetworkInterface::packet_dissector(const struct pcap_pkthdr *h, const u_char *packet) {
  struct ndpi_ethhdr *ethernet, dummy_ethernet;
  struct ndpi_iphdr *iph;
  u_int64_t time;
  static u_int64_t lasttime = 0;
  u_int16_t eth_type, ip_offset, vlan_id = 0;
  u_int32_t res = ntop->getGlobals()->get_detection_tick_resolution(), null_type;
  int pcap_datalink_type = get_datalink();

  setTimeLastPktRcvd(h->ts.tv_sec);

  time = ((uint64_t) h->ts.tv_sec) * res + h->ts.tv_usec / (1000000 / res);
  if(lasttime > time) time = lasttime;

  lasttime = time;

  if(pcap_datalink_type == DLT_NULL) {
    memcpy(&null_type, packet, sizeof(u_int32_t));

    switch(null_type) {
    case BSD_AF_INET:
      eth_type = ETHERTYPE_IP;
      break;
    case BSD_AF_INET6_BSD:
    case BSD_AF_INET6_FREEBSD:
    case BSD_AF_INET6_DARWIN:
      eth_type = ETHERTYPE_IPV6;
      break;
    default:
      incStats(0, NDPI_PROTOCOL_UNKNOWN, h->len, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
      return; /* Any other non IP protocol */
    }

    memset(&dummy_ethernet, 0, sizeof(dummy_ethernet));
    ethernet = (struct ndpi_ethhdr *)&dummy_ethernet;
    ip_offset = 4;
  } else if(pcap_datalink_type == DLT_EN10MB) {
    ethernet = (struct ndpi_ethhdr *) packet;
    ip_offset = sizeof(struct ndpi_ethhdr);
    eth_type = ntohs(ethernet->h_proto);
  } else if(pcap_datalink_type == 113 /* Linux Cooked Capture */) {
    memset(&dummy_ethernet, 0, sizeof(dummy_ethernet));
    ethernet = (struct ndpi_ethhdr *)&dummy_ethernet;
    eth_type = (packet[14] << 8) + packet[15];
    ip_offset = 16;
    incStats(0, NDPI_PROTOCOL_UNKNOWN, h->len, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
  } else {
    incStats(0, NDPI_PROTOCOL_UNKNOWN, h->len, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
    return;
  }

  if(eth_type == 0x8100 /* VLAN */) {
    Ether80211q *qType = (Ether80211q*)&packet[ip_offset];

    vlan_id = ntohs(qType->vlanId) & 0xFFF;
    eth_type = (packet[ip_offset+2] << 8) + packet[ip_offset+3];
    ip_offset += 4;
  }

  // just work on Ethernet packets that contain IPv4
  switch(eth_type) {
  case ETHERTYPE_IP:
    if(h->caplen >= ip_offset) {
      u_int16_t frag_off;

      iph = (struct ndpi_iphdr *) &packet[ip_offset];

      if(iph->version != 4) {
	/* This is not IPv4 */
	return;
      } else
	frag_off = ntohs(iph->frag_off);

      if(ntop->getGlobals()->decode_tunnels() && (iph->protocol == IPPROTO_UDP)
	 && ((frag_off & 0x3FFF /* IP_MF | IP_OFFSET */ ) == 0)) {
	u_short ip_len = ((u_short)iph->ihl * 4);
	struct ndpi_udphdr *udp = (struct ndpi_udphdr *)&packet[ip_offset+ip_len];
	u_int16_t sport = ntohs(udp->source), dport = ntohs(udp->dest);

	if((sport == GTP_U_V1_PORT) || (dport == GTP_U_V1_PORT)) {
	  /* Check if it's GTPv1 */
	  u_int offset = (u_int)(ip_offset+ip_len+sizeof(struct ndpi_udphdr));
	  u_int8_t flags = packet[offset];
	  u_int8_t message_type = packet[offset+1];

	  if((((flags & 0xE0) >> 5) == 1 /* GTPv1 */) && (message_type == 0xFF /* T-PDU */)) {
	    ip_offset = ip_offset+ip_len+sizeof(struct ndpi_udphdr)+8 /* GTPv1 header len */;

	    if(flags & 0x04) ip_offset += 1; /* next_ext_header is present */
	    if(flags & 0x02) ip_offset += 4; /* sequence_number is present (it also includes next_ext_header and pdu_number) */
	    if(flags & 0x01) ip_offset += 1; /* pdu_number is present */

	    iph = (struct ndpi_iphdr *) &packet[ip_offset];

	    if(iph->version != 4) {
	      /* FIX - Add IPv6 support */
	      return;
	    }
	  }
	}

      }

      packet_processing(h->ts.tv_sec, time, ethernet, vlan_id, iph, NULL, h->caplen - ip_offset, h->caplen);
    }
    break;

  case ETHERTYPE_IPV6:
    if(h->caplen >= ip_offset) {
      struct ndpi_ip6_hdr *ip6 = (struct ndpi_ip6_hdr*)&packet[ip_offset];

      if((ntohl(ip6->ip6_ctlun.ip6_un1.ip6_un1_flow) & 0xF0000000) != 0x60000000) {
	/* This is not IPv6 */
	return;
      } else
	packet_processing(h->ts.tv_sec, time, ethernet, vlan_id, NULL, ip6, h->len - ip_offset, h->len);
    }
    break;

  default: /* No IPv4 nor IPv6 */
    Host *srcHost = findHostByMac(ethernet->h_source, vlan_id, true);
    Host *dstHost = findHostByMac(ethernet->h_dest, vlan_id, true);

    if(srcHost) srcHost->incStats(0, NO_NDPI_PROTOCOL, 1, h->len, 0, 0);
    if(dstHost) dstHost->incStats(0, NO_NDPI_PROTOCOL, 0, 0, 1, h->len);

    incStats(eth_type, NDPI_PROTOCOL_UNKNOWN, h->len, 1, 24 /* 8 Preamble + 4 CRC + 12 IFG */);
    break;
  }

  purgeIdle(last_pkt_rcvd);
}

/* **************************************************** */

void NetworkInterface::startPacketPolling() {
  if (cpu_affinity >= 0) Utils::setThreadAffinity(pollLoop, cpu_affinity);
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Started packet polling on interface %s...", get_name());
  running = true;
}

/* **************************************************** */

void NetworkInterface::shutdown() {
  running = false;
}

/* **************************************************** */

void NetworkInterface::findFlowHosts(u_int16_t vlanId,
				     u_int8_t src_mac[6], IpAddress *_src_ip, Host **src,
				     u_int8_t dst_mac[6], IpAddress *_dst_ip, Host **dst) {

  (*src) = hosts_hash->get(vlanId, _src_ip);

  if((*src) == NULL) {
    (*src) = new Host(this, src_mac, vlanId, _src_ip);
    if(!hosts_hash->add(*src)) {
      //ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
      delete *src;
      *src = *dst = NULL;
      return;
    }
  }

  /* ***************************** */

  (*dst) = hosts_hash->get(vlanId, _dst_ip);

  if((*dst) == NULL) {
    (*dst) = new Host(this, dst_mac, vlanId, _dst_ip);
    if(!hosts_hash->add(*dst)) {
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
      delete *dst;
      *dst = NULL;
      return;
    }
  }
}

/* **************************************************** */

static void flow_sum_protos(GenericHashEntry *f, void *user_data) {
  NdpiStats *stats = (NdpiStats*)user_data;
  Flow *flow = (Flow*)f;

  flow->sumStats(stats);
}

/* **************************************************** */

void NetworkInterface::getnDPIStats(NdpiStats *stats) {
  memset(stats, 0, sizeof(NdpiStats));

  flows_hash->walk(flow_sum_protos, (void*)stats);
}

/* **************************************************** */

static void flow_update_hosts_stats(GenericHashEntry *node, void *user_data) {
  Flow *flow = (Flow*)node;
  struct timeval *tv = (struct timeval*)user_data;

  flow->update_hosts_stats(tv);
}

/* **************************************************** */

static void update_hosts_stats(GenericHashEntry *node, void *user_data) {
  GenericHost *host = (GenericHost*)node;
  struct timeval *tv = (struct timeval*)user_data;
  
  host->updateStats(tv);
  /*
  ntop->getTrace()->traceEvent(TRACE_WARNING, "Updated: %s [%d]", 
			       ((StringHost*)node)->host_key(),
			       host->getThptTrend());
  */
}

/* **************************************************** */

void NetworkInterface::updateHostStats() {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  flows_hash->walk(flow_update_hosts_stats, (void*)&tv);
  hosts_hash->walk(update_hosts_stats, (void*)&tv);
  strings_hash->walk(update_hosts_stats, (void*)&tv);
}

/* **************************************************** */

static void hosts_get_list(GenericHashEntry *h, void *user_data) {
  lua_State* vm = (lua_State*)user_data;

  ((Host*)h)->lua(vm, false, false, false);
}

/* **************************************************** */

static void hosts_get_list_details(GenericHashEntry *h, void *user_data) {
  lua_State* vm = (lua_State*)user_data;

  ((Host*)h)->lua(vm, true, false, false);
}

/* **************************************************** */

void NetworkInterface::getActiveHostsList(lua_State* vm, bool host_details) {
  lua_newtable(vm);

  hosts_hash->walk(host_details ? hosts_get_list_details : hosts_get_list, (void*)vm);
}

/* **************************************************** */

struct aggregation_walk_hosts_info {
  lua_State* vm;
  u_int16_t family_id;
};

/* **************************************************** */

static void aggregated_hosts_get_list(GenericHashEntry *h, void *user_data) {
  struct aggregation_walk_hosts_info *info = (struct aggregation_walk_hosts_info*)user_data;
  StringHost *host = (StringHost*)h;

  if((info->family_id == 0) || (info->family_id == host->get_family_id())) {
    ((StringHost*)h)->lua(info->vm, true);
  }
}

/* **************************************************** */

void NetworkInterface::getActiveAggregatedHostsList(lua_State* vm, u_int16_t proto_family) {
  struct aggregation_walk_hosts_info info;

  info.vm = vm, info.family_id = proto_family;

  lua_newtable(vm);

  strings_hash->walk(aggregated_hosts_get_list, (void*)&info);
}

/* **************************************************** */

struct host_find_info {
  char *host_to_find;
  u_int16_t vlan_id;
  Host *h;
  StringHost *s;
};

/* **************************************************** */

struct host_find_aggregation_info {
  IpAddress *host_to_find;
  lua_State* vm;
};

/* **************************************************** */

static void find_host_by_name(GenericHashEntry *h, void *user_data) {
  struct host_find_info *info = (struct host_find_info*)user_data;
  Host *host                  = (Host*)h;

  if((info->h == NULL)
     && (host->get_vlan_id() == info->vlan_id)
     && host->get_name()
     && (!strcmp(host->get_name(), info->host_to_find)))
    info->h = host;
}

/* **************************************************** */

static void find_aggregated_host_by_name(GenericHashEntry *h, void *user_data) {
  struct host_find_info *info = (struct host_find_info*)user_data;
  StringHost *host            = (StringHost*)h;

  if((info->s == NULL)
     && host->host_key()
     && info->host_to_find
     && (!strcmp(host->host_key(), info->host_to_find)))
    info->s = host;
}

/* **************************************************** */

static void find_aggregations_for_host_by_name(GenericHashEntry *h, void *user_data) {
  struct host_find_aggregation_info *info = (host_find_aggregation_info*)user_data;
  StringHost *host                        = (StringHost*)h;
  u_int n;
  
  if(!info->host_to_find) return;

  if((n = host->get_num_contacts_by(info->host_to_find)) > 0)
    lua_push_int_table_entry(info->vm, host->host_key(), n);
}

/* **************************************************** */

static void find_aggregation_families(GenericHashEntry *h, void *user_data) {
  NDPI_PROTOCOL_BITMASK *families = (NDPI_PROTOCOL_BITMASK*)user_data;
  StringHost *host                = (StringHost*)h;

  NDPI_ADD_PROTOCOL_TO_BITMASK(*families, host->get_family_id());
}

/* **************************************************** */

bool NetworkInterface::restoreHost(char *host_ip) {
  Host *h = new Host(this, host_ip);

  if(!h) return(false);

  if(!hosts_hash->add(h)) {
    //ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
    delete h;
    return(false);
  }

  return(true);
}

/* **************************************************** */

Host* NetworkInterface::getHost(char *host_ip, u_int16_t vlan_id) {
  struct in_addr  a4;
  struct in6_addr a6;
  Host *h = NULL;

  /* Check if address is invalid */
  if((inet_pton(AF_INET, (const char*)host_ip, &a4) == 0) && (inet_pton(AF_INET6, (const char*)host_ip, &a6) == 0)) {
    /* Looks like a symbolic name */
    struct host_find_info info;

    memset(&info, 0, sizeof(info));
    info.host_to_find = host_ip, info.vlan_id = vlan_id;
    hosts_hash->walk(find_host_by_name, (void*)&info);

    h = info.h;
  } else {
    IpAddress *ip = new IpAddress(host_ip);

    if(ip) {
      h = hosts_hash->get(vlan_id, ip);
      delete ip;
    }
  }

  return(h);
}

/* **************************************************** */

bool NetworkInterface::getHostInfo(lua_State* vm, char *host_ip, u_int16_t vlan_id) {
  Host *h = getHost(host_ip, vlan_id);

  if(h) {
    lua_newtable(vm);

    h->lua(vm, true, true, true);
    return(true);
  } else
    return(false);
}

/* **************************************************** */

bool NetworkInterface::getAggregatedHostInfo(lua_State* vm, char *host_name) {
  struct host_find_info info;

  memset(&info, 0, sizeof(info));
  info.host_to_find = host_name;
  strings_hash->walk(find_aggregated_host_by_name, (void*)&info);

  if(info.s != NULL) {
    lua_newtable(vm);

    info.s->lua(vm, false);
    return(true);
  } else
    return(false);
}

/* **************************************************** */

/*
  Returns all aggregations that have the given host as requestor

  Example if we are looking at the DNS requests, it will return all DNS
  names requested by host X (host_name)
*/
bool NetworkInterface::getAggregationsForHost(lua_State* vm, char *host_ip) {
  struct host_find_aggregation_info info;
  IpAddress *h = new IpAddress(host_ip);

  memset(&info, 0, sizeof(info));
  info.host_to_find = h, info.vm = vm;

  lua_newtable(vm);
  strings_hash->walk(find_aggregations_for_host_by_name, (void*)&info);
  delete h;
  
  return(true);
}

/* **************************************************** */

bool NetworkInterface::getAggregatedFamilies(lua_State* vm) {
  NDPI_PROTOCOL_BITMASK families;

  NDPI_BITMASK_RESET(families);
  strings_hash->walk(find_aggregation_families, (void*)&families);

  lua_newtable(vm);
  
  for(int i=0; i<(NDPI_LAST_IMPLEMENTED_PROTOCOL+NDPI_MAX_NUM_CUSTOM_PROTOCOLS); i++)
    if(NDPI_COMPARE_PROTOCOL_TO_BITMASK(families, i)) {
      lua_push_int_table_entry(vm, ndpi_get_proto_name(strings_hash->getInterface()->get_ndpi_struct(), i), i);
    }

  return(true);
}

/* **************************************************** */

static void flows_get_list_details(GenericHashEntry *h, void *user_data) {
  lua_State* vm = (lua_State*)user_data;
  Flow *flow = (Flow*)h;

  flow->lua(vm, false /* Minimum details */);
}

/* **************************************************** */

void NetworkInterface::getActiveFlowsList(lua_State* vm) {
  lua_newtable(vm);

  flows_hash->walk(flows_get_list_details, (void*)vm);
}

/* **************************************************** */

struct flow_peers_info {
  lua_State *vm;
  char *numIP;
};

static void flow_peers_walker(GenericHashEntry *h, void *user_data) {
  Flow *flow = (Flow*)h;
  struct flow_peers_info *info = (struct flow_peers_info*)user_data;

  if((info->numIP == NULL) || flow->isFlowPeer(info->numIP))
    flow->print_peers(info->vm, (info->numIP == NULL) ? false : true);
}

/* **************************************************** */

void NetworkInterface::getFlowPeersList(lua_State* vm, char *numIP) {
  struct flow_peers_info info;

  lua_newtable(vm);

  info.vm = vm, info.numIP = numIP;

  flows_hash->walk(flow_peers_walker, (void*)&info);
}

/* **************************************************** */

int ptr_compare(const void *a, const void *b) {
  if(a == b)
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Flow found");
  return((a == b) ? 0 : 1);
}

/* **************************************************** */

u_int NetworkInterface::purgeIdleFlows() {
  if(!purge_idle_flows_hosts) return(0);

  if(next_idle_flow_purge == 0) {
    next_idle_flow_purge = last_pkt_rcvd + FLOW_PURGE_FREQUENCY;
    return(0);
  } else if(last_pkt_rcvd < next_idle_flow_purge)
    return(0); /* Too early */
  else {
    /* Time to purge flows */
    u_int n;

    // ntop->getTrace()->traceEvent(TRACE_INFO, "Purging idle flows");
    n = flows_hash->purgeIdle();
    next_idle_flow_purge = last_pkt_rcvd + FLOW_PURGE_FREQUENCY;
    return(n);
  }
}

/* **************************************************** */

u_int NetworkInterface::getNumFlows() { return(flows_hash->getNumEntries()); };
u_int NetworkInterface::getNumHosts() { return(hosts_hash->getNumEntries()); };

/* **************************************************** */

u_int NetworkInterface::purgeIdleHosts() {
  if(!purge_idle_flows_hosts) return(0);

  if(next_idle_host_purge == 0) {
    next_idle_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
    return(0);
  } else if(last_pkt_rcvd < next_idle_host_purge)
    return(0); /* Too early */
  else {
    /* Time to purge hosts */
    u_int n;

    // ntop->getTrace()->traceEvent(TRACE_INFO, "Purging idle hosts");
    n = hosts_hash->purgeIdle();
    next_idle_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
    return(n);
  }
}

/* **************************************************** */

u_int NetworkInterface::purgeIdleAggregatedHosts() {
  if(!purge_idle_flows_hosts) return(0);

  if(next_idle_aggregated_host_purge == 0) {
    next_idle_aggregated_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
    return(0);
  } else if(last_pkt_rcvd < next_idle_aggregated_host_purge)
    return(0); /* Too early */
  else {
    /* Time to purge hosts */
    u_int n;

    // ntop->getTrace()->traceEvent(TRACE_INFO, "Purging idle aggregated hosts");
    n = strings_hash->purgeIdle();
    next_idle_aggregated_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
    return(n);
  }
}

/* *************************************** */

void NetworkInterface::lua(lua_State *vm) {
  lua_newtable(vm);

  lua_push_str_table_entry(vm, "name", ifname);
  lua_push_str_table_entry(vm, "type", (char*)get_type());

  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[%s][EthStats][Rcvd: %llu]", ifname, getNumPackets());
  lua_push_int_table_entry(vm, "stats_packets", getNumPackets());
  lua_push_int_table_entry(vm, "stats_bytes",   getNumBytes());
  lua_push_int_table_entry(vm, "stats_flows", getNumFlows());
  lua_push_int_table_entry(vm, "stats_hosts", getNumHosts());
  lua_push_int_table_entry(vm, "stats_drops", getNumDrops());

  ethStats.lua(vm);
  ndpiStats.lua(this, vm);
  pktStats.lua(vm, "pktSizeDistribution");

  lua_pushinteger(vm, 0); //  Index
  lua_insert(vm, -2);
}

/* **************************************************** */

void NetworkInterface::runHousekeepingTasks() {
  // NdpiStats stats;

  /* TO COMPLETE */
  updateHostStats();
  // getnDPIStats(&stats);
  //stats.print(iface);
  //dumpFlows();
}

/* **************************************************** */

Host* NetworkInterface::findHostByMac(u_int8_t mac[6], u_int16_t vlanId,
				      bool createIfNotPresent) {
  Host *ret = hosts_hash->get(vlanId, mac);

  if((ret == NULL) && createIfNotPresent) {
    if((ret = new Host(this, mac, vlanId)) != NULL)
      hosts_hash->add(ret);
  }

  return(ret);
}

/* **************************************************** */

Flow* NetworkInterface::findFlowByKey(u_int32_t key) {
  return((Flow*)(flows_hash->findByKey(key)));
}

/* **************************************************** */

bool NetworkInterface::validInterface(char *name) {
  if(name &&
     (strstr(name, "PPP")         /* Avoid to use the PPP interface              */
      || strstr(name, "dialup")   /* Avoid to use the dialup interface           */
      || strstr(name, "ICSHARE")  /* Avoid to use the internet sharing interface */
      || strstr(name, "NdisWan"))) { /* Avoid to use the internet sharing interface */
    return(false);
  }

  return(true);
}

/* **************************************************** */

void NetworkInterface::printAvailableInterfaces(bool printHelp, int idx, char *ifname, u_int ifname_len) {
  char ebuf[256];
  int i, numInterfaces = 0;
  pcap_if_t *devpointer;

  if(help_printed) return; else help_printed = true;

  ebuf[0] = '\0';

  if(ifname == NULL) {
    if(printHelp)
      printf("Available interfaces (-i <interface index>):\n");
    else
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "Available interfaces (-i <interface index>):");
  }

  if(pcap_findalldevs(&devpointer, ebuf) < 0) {
    ;
  } else {
    for(i = 0; devpointer != 0; i++) {
      if(validInterface(devpointer->description)) {
	numInterfaces++;

	if(ifname == NULL) {
	  if(printHelp) {
#ifdef WIN32
	    printf("   %d. %s\n"
		   "\t%s\n", numInterfaces,
		   devpointer->description ? devpointer->description : "",
		   devpointer->name);
#else
	    printf("   %d. %s\n", numInterfaces, devpointer->name);
#endif
	  } else
	    ntop->getTrace()->traceEvent(TRACE_NORMAL, " %d. %s (%s)\n",
					 numInterfaces,
					 devpointer->description ? devpointer->description : "",
					 devpointer->name);
	} else if(numInterfaces == idx) {
	  snprintf(ifname, ifname_len, "%s", devpointer->name);
	  break;
	}
      }

      devpointer = devpointer->next;
    } /* for */
  } /* else */

  if(numInterfaces == 0) {
#ifdef WIN32
    ntop->getTrace()->traceEvent(TRACE_WARNING, "No interfaces available! This application cannot work");
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Make sure that winpcap is installed properly,");
    ntop->getTrace()->traceEvent(TRACE_WARNING, "that you have administrative rights,");
    ntop->getTrace()->traceEvent(TRACE_WARNING, "and that you have network interfaces installed.");
#else
    ntop->getTrace()->traceEvent(TRACE_WARNING, "No interfaces available: are you superuser?");
#endif
  }
}

/* **************************************************** */

bool NetworkInterface::isNumber(const char *str) {
  while(*str) {
    if(!isdigit(*str))
      return(false);

    str++;
  }

  return(true);
}

/* **************************************************** */

StringHost* NetworkInterface::findHostByString(char *keyname,
					       u_int16_t family_id,
					       bool createIfNotPresent) {
  StringHost *ret = strings_hash->get(keyname);

  if((ret == NULL) && createIfNotPresent) {
    if((ret = new StringHost(this, keyname, family_id)) != NULL)
      strings_hash->add(ret);
  }

  return(ret);
}
