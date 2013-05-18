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

NetworkInterface::NetworkInterface(char *name, bool change_user) {
  char pcap_error_buffer[PCAP_ERRBUF_SIZE];
  NDPI_PROTOCOL_BITMASK all;

  if(name == NULL) {
    name = pcap_lookupdev(pcap_error_buffer);

    if(name == NULL) {
      printf("ERROR: Unable to locate default interface (%s)\n", pcap_error_buffer);
      exit(0);
    }
  }

  ifname = strdup(name);

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

  flows_hash = new FlowHash(4096, 32768), hosts_hash = new HostHash(4096, 32768);

  // init global detection structure
  ndpi_struct = ndpi_init_detection_module(ntop->getGlobals()->get_detection_tick_resolution(),
					   malloc_wrapper, free_wrapper, debug_printf);
  if(ndpi_struct == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Global structure initialization failed");
    exit(-1);
  }

  // enable all protocols
  NDPI_BITMASK_SET_ALL(all);
  ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);
  
  next_idle_flow_purge = next_idle_host_purge = 0;
  polling_started = false;
}

/* **************************************************** */

NetworkInterface::~NetworkInterface() {
  if(polling_started) {
    void *res;

    if(pcap_handle) pcap_breakloop(pcap_handle);
    pthread_join(pollLoop, &res);
  }

  if(pcap_handle)
    pcap_close(pcap_handle);

  delete flows_hash;
  delete hosts_hash;

  ndpi_exit_detection_module(ndpi_struct, free_wrapper);
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Interface %s shutdown", ifname);
  free(ifname);
}

/* **************************************************** */

static void node_proto_guess_walker(HashEntry *node, void *user_data) {
  Flow *flow = (Flow*)node;

  flow->print();
}

/* **************************************************** */

void NetworkInterface::dumpFlows() {
  flows_hash->walk(node_proto_guess_walker, NULL);
}

/* **************************************************** */

Flow* NetworkInterface::getFlow(struct ndpi_ethhdr *eth, u_int16_t vlan_id,
				struct ndpi_iphdr *iph, 
				struct ndpi_ip6_hdr *ip6,
				u_int16_t ipsize, bool *src2dst_direction,
				u_int8_t *l4_proto) {
  u_int16_t l4_packet_len;
  struct ndpi_tcphdr *tcph = NULL;
  struct ndpi_udphdr *udph = NULL;
  u_int16_t src_port, dst_port;
  Flow *ret;
  u_int8_t *l4;

  if(iph != NULL) {
    /* IPv4 */
    if(ipsize < 20)
      return NULL;

    if((iph->ihl * 4) > ipsize || ipsize < ntohs(iph->tot_len)
       || (iph->frag_off & htons(0x1FFF)) != 0) {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "IPv4 fragments are not handled yet");
      return NULL;
    }

    l4_packet_len = ntohs(iph->tot_len) - (iph->ihl * 4);
    *l4_proto = iph->protocol;
    l4 = ((u_int8_t *) iph + iph->ihl * 4);
  } else {
    /* IPv6 */

    if(ipsize < sizeof(const struct ndpi_ip6_hdr))
      return NULL;
    
    l4_packet_len = ntohs(ip6->ip6_ctlun.ip6_un1.ip6_un1_plen)-sizeof(const struct ndpi_ip6_hdr);
    *l4_proto = ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    l4 = (u_int8_t*)ip6 + sizeof(const struct ndpi_ip6_hdr);
  }

  if((*l4_proto == IPPROTO_TCP) && (l4_packet_len >= 20)) {
    // tcp
    tcph = (struct ndpi_tcphdr *)l4;
    src_port = tcph->source, dst_port = tcph->dest;
  } else if((*l4_proto == IPPROTO_UDP) && (l4_packet_len >= 8)) {
    // udp
    udph = (struct ndpi_udphdr *)l4;
    src_port = udph->source,  dst_port = udph->dest;
  } else {
    // non tcp/udp protocols
    return(NULL);
  }

  ret = flows_hash->find(iph, ip6, src_port, dst_port, vlan_id, *l4_proto, src2dst_direction);

  if(ret == NULL) {
    if(iph)
      ret = new Flow(this, vlan_id, *l4_proto, 
		     (u_int8_t*)eth->h_source, iph->saddr, NULL, src_port, 
		     (u_int8_t*)eth->h_dest, iph->daddr, NULL, dst_port);
    else
      ret = new Flow(this, vlan_id, *l4_proto, 
		     (u_int8_t*)eth->h_source, 0, &ip6->ip6_src, src_port, 
		     (u_int8_t*)eth->h_dest, 0, &ip6->ip6_dst, dst_port);
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

void NetworkInterface::packet_processing(const u_int64_t time, 
					 struct ndpi_ethhdr *eth,
					 u_int16_t vlan_id,
					 struct ndpi_iphdr *iph,
					 struct ndpi_ip6_hdr *ip6,
					 u_int16_t ipsize, u_int16_t rawsize)
{
  bool src2dst_direction;
  u_int8_t l4_proto;
  Flow *flow = getFlow(eth, vlan_id, iph, ip6, ipsize, &src2dst_direction, &l4_proto);
  u_int16_t frag_off = iph ? ntohs(iph->frag_off) : 0;
  u_int8_t *ip;

  if(flow == NULL) return; else flow->incStats(src2dst_direction, rawsize);
  if(flow->isDetectionCompleted()) return;

  if((frag_off & 0x3FFF) == 0) {
    struct ndpi_flow_struct *ndpi_flow = flow->get_ndpi_flow();
    struct ndpi_id_struct *src = (struct ndpi_id_struct*)flow->get_src_id();
    struct ndpi_id_struct *dst = (struct ndpi_id_struct*)flow->get_dst_id();

    if(iph != NULL)
      ip = (u_int8_t*)iph;
    else
      ip = (u_int8_t*)ip6;

    flow->setDetectedProtocol(ndpi_detection_process_packet(ndpi_struct, ndpi_flow,
							    ip, ipsize, time, src, dst),
			      l4_proto);
  } else {
    // FIX - only handle unfragmented packets  
    ntop->getTrace()->traceEvent(TRACE_WARNING, "IP fragments are not handled yet!");
  }
}

/* **************************************************** */

static void pcap_packet_callback(u_char * args, const struct pcap_pkthdr *h, const u_char * packet) {
  NetworkInterface *iface = (NetworkInterface*)args;
  struct ndpi_ethhdr *ethernet;
  struct ndpi_iphdr *iph;
  u_int64_t time;
  static u_int64_t lasttime = 0;
  u_int16_t type, ip_offset, vlan_id = 0;
  u_int32_t res = ntop->getGlobals()->get_detection_tick_resolution();
  int pcap_datalink_type = iface->get_datalink();

  time = ((uint64_t) h->ts.tv_sec) * res + h->ts.tv_usec / (1000000 / res);
  if(lasttime > time) time = lasttime;

  lasttime = time;

  if(pcap_datalink_type == DLT_EN10MB) {
    ethernet = (struct ndpi_ethhdr *) packet;
    ip_offset = sizeof(struct ndpi_ethhdr);
    type = ntohs(ethernet->h_proto);
  } else if(pcap_datalink_type == 113 /* Linux Cooked Capture */) {
    type = (packet[14] << 8) + packet[15];
    ip_offset = 16;
    iface->incStats(h->ts.tv_sec, 0, h->caplen);
  } else {
    iface->incStats(h->ts.tv_sec, 0, h->caplen);
    return;
  }

  if(type == 0x8100 /* VLAN */) {
    Ether80211q *qType = (Ether80211q*)&packet[ip_offset];

    vlan_id = ntohs(qType->vlanId) & 0xFFF;
    type = (packet[ip_offset+2] << 8) + packet[ip_offset+3];
    ip_offset += 4;
  }

  iface->incStats(h->ts.tv_sec, type, h->caplen);

  // just work on Ethernet packets that contain IPv4
  switch(type) {
  case ETHERTYPE_IP:
    if(h->caplen >= ip_offset) {
      u_int16_t frag_off;

      iph = (struct ndpi_iphdr *) &packet[ip_offset];

      if(iph->version != 4) {
	/* This is not IPv4 */
	return;
      } else
	frag_off = ntohs(iph->frag_off);

      if(ntop->getGlobals()->decode_tunnels() && (iph->protocol == IPPROTO_UDP) && ((frag_off & 0x3FFF) == 0)) {
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

      iface->packet_processing(time, ethernet, vlan_id, iph, NULL, h->len - ip_offset, h->len);
    }
    break;

  case ETHERTYPE_IPV6:
    if(h->caplen >= ip_offset) {
      struct ndpi_ip6_hdr *ip6 = (struct ndpi_ip6_hdr*)&packet[ip_offset];

      if((ntohl(ip6->ip6_ctlun.ip6_un1.ip6_un1_flow) & 0xF0000000) != 0x60000000) {
	/* This is not IPv6 */
	return;
      } else
	iface->packet_processing(time, ethernet, vlan_id, NULL, ip6, h->len - ip_offset, h->len);
    }
    break;
    
  default: /* No IPv4 nor IPv6 */
    Host *srcHost = iface->findHostByMac((const u_int8_t*)ethernet->h_source, true);
    Host *dstHost = iface->findHostByMac((const u_int8_t*)ethernet->h_dest, true);

    if(srcHost) srcHost->incStats(NO_NDPI_PROTOCOL, 1, h->len, 0, 0);
    if(dstHost) dstHost->incStats(NO_NDPI_PROTOCOL, 0, 0, 1, h->len);
    break;
  }

  iface->purgeIdleFlows();
  iface->purgeIdleHosts();
}

/* **************************************************** */

static void* packetPollLoop(void* ptr) {
  NetworkInterface *iface = (NetworkInterface*)ptr;

  pcap_loop(iface->get_pcap_handle(), -1, &pcap_packet_callback, (u_char*)iface);
  return(NULL);
}

/* **************************************************** */

void NetworkInterface::startPacketPolling() {
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Started packet polling...");

  pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);
  polling_started = true;
}

/* **************************************************** */

void NetworkInterface::shutdown() {
  pcap_breakloop(pcap_handle);
}

/* **************************************************** */

void NetworkInterface::findFlowHosts(u_int8_t src_mac[6], u_int32_t _src_ipv4, struct ndpi_in6_addr *_src_ipv6, Host **src, 
				     u_int8_t dst_mac[6], u_int32_t _dst_ipv4, struct ndpi_in6_addr *_dst_ipv6, Host **dst) {
  IpAddress ip;

  if(_src_ipv6) ip.set_ipv6(_src_ipv6); else ip.set_ipv4(_src_ipv4);
  (*src) = hosts_hash->get(&ip);

  if((*src) == NULL) {
    (*src) = new Host(this, src_mac, &ip);
    if(!hosts_hash->add(*src)) {
      //ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
      delete *src;
      *src = *dst = NULL;
      return;
    }
  }

  /* ***************************** */

  if(_dst_ipv6) ip.set_ipv6(_dst_ipv6); else ip.set_ipv4(_dst_ipv4);
  (*dst) = hosts_hash->get(&ip);

  if((*dst) == NULL) {
    (*dst) = new Host(this, dst_mac, &ip);
    if(!hosts_hash->add(*dst)) {
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
      delete *dst;
      *dst = NULL;
      return;
    }
  }
}

/* **************************************************** */

static void hosts_node_sum_protos(HashEntry *h, void *user_data) {
  NdpiStats *stats = (NdpiStats*)user_data;

  ((Host*)h)->get_ndpi_stats()->sumStats(stats);
}

/* **************************************************** */

void NetworkInterface::getnDPIStats(NdpiStats *stats) {
  memset(stats, 0, sizeof(NdpiStats));

  hosts_hash->walk(hosts_node_sum_protos, (void*)stats);
}

/* **************************************************** */

static void flow_update_hosts_stats(HashEntry *node, void *user_data) {
  Flow *flow = (Flow*)node;

  flow->update_hosts_stats();
}

/* **************************************************** */

void NetworkInterface::updateHostStats() {
  flows_hash->walk(flow_update_hosts_stats, this);
}

/* **************************************************** */

static void hosts_get_list(HashEntry *h, void *user_data) {
  lua_State* vm = (lua_State*)user_data;

  ((Host*)h)->lua(vm, false, false);
}

/* **************************************************** */

static void hosts_get_list_details(HashEntry *h, void *user_data) {
  lua_State* vm = (lua_State*)user_data;

  ((Host*)h)->lua(vm, true, false);
}

/* **************************************************** */

void NetworkInterface::getActiveHostsList(lua_State* vm, bool host_details) {
  lua_newtable(vm);

  hosts_hash->walk(host_details ? hosts_get_list_details : hosts_get_list, (void*)vm);
}

/* **************************************************** */

bool NetworkInterface::getHostInfo(lua_State* vm, char *host_ip) {
  struct in_addr  a4;
  struct in6_addr a6;

  /* Check if address is invalid */
  if(inet_pton(AF_INET, (const char*)host_ip, &a4)
     || inet_pton(AF_INET6, (const char*)host_ip, &a6)) {
    IpAddress *ip = new IpAddress(host_ip);
    bool found = false;

    if(ip) {
      Host *h = hosts_hash->get(ip);

      if(ip && h) {
	lua_newtable(vm);
      
	h->lua(vm, true, true);
	found = true;
      }

      delete ip;
    }

    return(found);
  } else
    return(false);
}

/* **************************************************** */

static void flows_get_list_details(HashEntry *h, void *user_data) {
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

static void flow_peers_walker(HashEntry *h, void *user_data) {
  Flow *flow = (Flow*)h;

  flow->print_peers((lua_State*)user_data);
}

/* **************************************************** */

void NetworkInterface::getFlowPeersList(lua_State* vm) {
  lua_newtable(vm);

  flows_hash->walk(flow_peers_walker, (void*)vm);
}

/* **************************************************** */

int ptr_compare(const void *a, const void *b) {
  if(a == b)
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Flow found");
  return((a == b) ? 0 : 1);
}

/* **************************************************** */
/* **************************************************** */

void NetworkInterface::purgeIdleFlows() {
  if(next_idle_flow_purge == 0) {
    next_idle_flow_purge = last_pkt_rcvd + FLOW_PURGE_FREQUENCY;
    return;
  } else if(last_pkt_rcvd < next_idle_flow_purge)
    return; /* Too early */
  else {
    /* Time to purge flows */
    ntop->getTrace()->traceEvent(TRACE_INFO, "Purging idle flows");
    flows_hash->purgeIdle();
    next_idle_flow_purge = last_pkt_rcvd + FLOW_PURGE_FREQUENCY;
  }
}

/* **************************************************** */

u_int NetworkInterface::getNumFlows() { return(flows_hash->getNumEntries()); };
u_int NetworkInterface::getNumHosts() { return(hosts_hash->getNumEntries()); };

/* **************************************************** */

void NetworkInterface::purgeIdleHosts() {
  if(next_idle_host_purge == 0) {
    next_idle_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
    return;
  } else if(last_pkt_rcvd < next_idle_host_purge)
    return; /* Too early */
  else {
    /* Time to purge hosts */
    ntop->getTrace()->traceEvent(TRACE_INFO, "Purging idle hosts");
    //    hosts_hash->purgeIdle();
    next_idle_host_purge = last_pkt_rcvd + HOST_PURGE_FREQUENCY;
  }
}

/* **************************************************** */

void NetworkInterface::dropPrivileges() {
#ifndef WIN32
  struct passwd *pw = NULL;
  const char *username;

  if(getgid() && getuid()) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Privileges are not dropped as we're not superuser");
    return;
  }
  
  username = "nobody";
  pw = getpwnam(username);
  
  if(pw == NULL) {
    username = "anonymous";
    pw = getpwnam(username);
  }

  if(pw != NULL) {
    /* Drop privileges */
    if((setgid(pw->pw_gid) != 0) || (setuid(pw->pw_uid) != 0)) {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to drop privileges [%s]",
		 strerror(errno));
    } else
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "User changeod to %s", username);
  } else {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to locate user %s", username);
  }

  umask(0);
#endif
}

/* *************************************** */

void NetworkInterface::lua(lua_State *vm) {
  lua_newtable(vm);
  lua_push_str_table_entry(vm, "name", ifname);
 
  lua_push_int_table_entry(vm, "stats_packets", getNumPackets());
  lua_push_int_table_entry(vm, "stats_bytes",   getNumBytes());
  lua_push_int_table_entry(vm, "stats_flows", getNumFlows());
  lua_push_int_table_entry(vm, "stats_hosts", getNumHosts());

  ethStats.lua(vm);
 
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

Host* NetworkInterface::findHostByMac(const u_int8_t mac[6], bool createIfNotPresent) {
  static u_int8_t printed = 0;

  if(!printed) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s() NOT IMPLEMENTED", __FUNCTION__);
    printed = 1;
  }

  return(NULL);
}
