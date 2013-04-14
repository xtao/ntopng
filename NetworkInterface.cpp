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

#ifndef ETH_P_IP
#define ETH_P_IP 0x0800
#endif

#define GTP_U_V1_PORT              2152
#define MAX_NUM_INTERFACE_HOSTS   65536

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
  ifStats = new TrafficStats();

#ifndef WIN32
  if(strstr(ifname, ".pcap") != NULL) {

    printf("ERROR: could not open pcap file: %s\n", pcap_error_buffer);

  }
#endif

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

  delete ifStats;

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

Flow* NetworkInterface::getFlow(u_int16_t vlan_id, const struct ndpi_iphdr *iph, u_int16_t ipsize, bool *src2dst_direction) {
  u_int16_t l4_packet_len;
  struct ndpi_tcphdr *tcph = NULL;
  struct ndpi_udphdr *udph = NULL;
  u_int32_t src_ip;
  u_int32_t dst_ip;
  u_int16_t src_port, sport;
  u_int16_t dst_port, dport;
  Flow *ret;

  if(ipsize < 20)
    return NULL;

  if((iph->ihl * 4) > ipsize || ipsize < ntohs(iph->tot_len)
     || (iph->frag_off & htons(0x1FFF)) != 0)
    return NULL;

  l4_packet_len = ntohs(iph->tot_len) - (iph->ihl * 4);

  if(iph->saddr < iph->daddr) {
    src_ip = iph->saddr;
    dst_ip = iph->daddr;
  } else {
    src_ip = iph->daddr;
    dst_ip = iph->saddr;
  }

  if(iph->protocol == 6 && l4_packet_len >= 20) {
    // tcp
    tcph = (struct ndpi_tcphdr *) ((u_int8_t *) iph + iph->ihl * 4);
    sport = tcph->source, dport = tcph->dest;

    if(iph->saddr < iph->daddr) {
      src_port = tcph->source;
      dst_port = tcph->dest;
    } else {
      src_port = tcph->dest;
      dst_port = tcph->source;
    }
  } else if(iph->protocol == 17 && l4_packet_len >= 8) {
    // udp
    udph = (struct ndpi_udphdr *) ((u_int8_t *) iph + iph->ihl * 4);
    sport = udph->source, dport = udph->dest;

    if(iph->saddr < iph->daddr) {
      src_port = udph->source;
      dst_port = udph->dest;
    } else {
      src_port = udph->dest;
      dst_port = udph->source;
    }
  } else {
    // non tcp/udp protocols
    src_port = 0;
    dst_port = 0;
  }

  ret = flows_hash->find(src_ip, dst_ip, src_port, dst_port, iph->protocol, vlan_id);

  if(ret == NULL) {
    ret = new Flow(this, vlan_id, iph->protocol, src_ip, src_port, dst_ip, dst_port);
    if(flows_hash->add(ret)) {
      *src2dst_direction = true;
      return(ret);
    } else {
      delete ret;
      // ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many flows");
      return(NULL); 
    }
  } else {
    if((ret->get_src_ipv4() == iph->saddr) && (ret->get_dst_ipv4() == iph->daddr)
       && (ret->get_src_port() == sport) && (ret->get_dst_port() == dport))
      *src2dst_direction = true;
    else
      *src2dst_direction = false;

    return(ret);
  }
}

/* **************************************************** */

void NetworkInterface::packet_processing(const u_int64_t time, u_int16_t vlan_id,
					 const struct ndpi_iphdr *iph,
					 u_int16_t ipsize, u_int16_t rawsize)
{
  bool src2dst_direction;
  Flow *flow = getFlow(vlan_id, iph, ipsize, &src2dst_direction);
  u_int16_t frag_off = ntohs(iph->frag_off);

  if(flow == NULL) return; else flow->incStats(src2dst_direction, rawsize);
  if(flow->isDetectionCompleted()) return;

  // only handle unfragmented packets
  if((frag_off & 0x3FFF) == 0) {
    struct ndpi_flow_struct *ndpi_flow = flow->get_ndpi_flow();
    struct ndpi_id_struct *src = (struct ndpi_id_struct*)flow->get_src_id();
    struct ndpi_id_struct *dst = (struct ndpi_id_struct*)flow->get_dst_id();

    flow->setDetectedProtocol(ndpi_detection_process_packet(ndpi_struct,
							    ndpi_flow, (uint8_t *)iph, ipsize, time, src, dst),
			      iph->protocol);
  }
}

/* **************************************************** */

static void pcap_packet_callback(u_char * args, const struct pcap_pkthdr *h, const u_char * packet) {
  NetworkInterface *iface = (NetworkInterface*)args;
  const struct ndpi_ethhdr *ethernet;
  struct ndpi_iphdr *iph;
  u_int64_t time;
  static u_int64_t lasttime = 0;
  u_int16_t type, ip_offset, vlan_id = 0;
  u_int32_t res = ntop->getGlobals()->get_detection_tick_resolution();
  int pcap_datalink_type = iface->get_datalink();

  iface->incStats(h->ts.tv_sec, h->caplen);

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
  } else
    return;

  if(type == 0x8100 /* VLAN */) {
    Ether80211q *qType = (Ether80211q*)&packet[ip_offset];
    vlan_id = ntohs(qType->vlanId) & 0xFFF;

    type = (packet[ip_offset+2] << 8) + packet[ip_offset+3];
    ip_offset += 4;
  }

  iph = (struct ndpi_iphdr *) &packet[ip_offset];

  // just work on Ethernet packets that contain IP
  if(type == ETH_P_IP && h->caplen >= ip_offset) {
    u_int16_t frag_off = ntohs(iph->frag_off);

    if(iph->version != 4) {
      /* FIX - Add IPv6 support */
      return;
    }

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

    iface->packet_processing(time, vlan_id, iph, h->len - ip_offset, h->len);
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

void NetworkInterface::findFlowHosts(Flow *flow, Host **src, Host **dst) {
  IpAddress ip;

  // FIX - Add IPv6 support
  ip.set_ipv4(flow->get_src_ipv4());
  (*src) = hosts_hash->get(&ip);

  if((*src) == NULL) {
    (*src) = new Host(this, flow->get_src_ipv4());
    if(!hosts_hash->add(*src)) {
      //ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many hosts in interface %s", ifname);
      delete *src;
      *src = *dst = NULL;
      return;
    }
  }

  /* ***************************** */

  ip.set_ipv4(flow->get_dst_ipv4());
  (*dst) = hosts_hash->get(&ip);

  if((*dst) == NULL) {
    (*dst) = new Host(this, flow->get_dst_ipv4());
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

  ((Host*)h)->dumpKeyToLua(vm);
}

/* **************************************************** */

void NetworkInterface::getActiveHostsList(lua_State* vm) {
  lua_newtable(vm);

  hosts_hash->walk(hosts_get_list, (void*)vm);
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

static void idle_flow_walker(HashEntry *h, void *user_data) {
  Flow *flow = (Flow*)h;
  NetworkInterface *iface = (NetworkInterface*)user_data;

  if(flow->isIdle(FLOW_MAX_IDLE)) {
    ntop->getTrace()->traceEvent(TRACE_INFO, "Delete idle flow");
    iface->removeFlow(flow, false /* Don't double lock */);
    delete flow;
  }
}

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
    flows_hash->walk(idle_flow_walker, (void*)this);
    next_idle_flow_purge = last_pkt_rcvd + FLOW_PURGE_FREQUENCY;
  }
}

/* **************************************************** */

bool NetworkInterface::removeFlow(Flow *flow, bool lock_hash) {  return(flows_hash->remove(flow, lock_hash)); };

/* **************************************************** */

bool NetworkInterface::removeHost(Host *host, bool lock_hash) {  return(hosts_hash->remove(host, lock_hash)); };

/* **************************************************** */

u_int NetworkInterface::getNumFlows() { return(flows_hash->getNumEntries()); };

/* **************************************************** */

u_int NetworkInterface::getNumHosts() { return(hosts_hash->getNumEntries()); };

/* **************************************************** */

static void idle_host_walker(HashEntry *h, void *user_data) {
  Host *host = (Host*)h;
  NetworkInterface *iface = (NetworkInterface*)user_data;

  if(host->isIdle(HOST_MAX_IDLE)) {
    ntop->getTrace()->traceEvent(TRACE_INFO, "Delete idle host");
    iface->removeHost(host, false /* Don't double lock */);
    delete host;
  }
}

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
    hosts_hash->walk(idle_host_walker, (void*)this);
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
