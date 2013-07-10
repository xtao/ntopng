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

Flow::Flow(NetworkInterface *_iface,
	   u_int16_t _vlanId, u_int8_t _protocol, 
	   u_int8_t src_mac[6], IpAddress *_src_ip, u_int16_t _src_port,
	   u_int8_t dst_mac[6], IpAddress *_dst_ip, u_int16_t _dst_port,
	   time_t _first_seen, time_t _last_seen) : GenericHashEntry(_iface) {
  vlanId = _vlanId, protocol = _protocol, src_port = _src_port, dst_port = _dst_port;
  cli2srv_packets = 0, cli2srv_bytes = 0, srv2cli_packets = 0, srv2cli_bytes = 0, cli2srv_last_packets = 0,
    cli2srv_last_bytes = 0, srv2cli_last_packets = 0, srv2cli_last_bytes = 0;
  
  detection_completed = false, detected_protocol = NDPI_PROTOCOL_UNKNOWN;
  ndpi_flow = NULL, src_id = dst_id = NULL;
  json_info = strdup("{}");

  iface->findFlowHosts(_vlanId, src_mac, _src_ip, &src_host, dst_mac, _dst_ip, &dst_host);
  if(src_host) src_host->incUses();
  if(dst_host) dst_host->incUses();
  first_seen = _first_seen;
  last_seen = _last_seen;
  categorization.category = NULL, categorization.flow_categorized = false;
  allocFlowMemory();
}

/* *************************************** */

Flow::Flow(NetworkInterface *_iface,
	   u_int16_t _vlanId, u_int8_t _protocol, 
	   u_int8_t src_mac[6], IpAddress *_src_ip, u_int16_t _src_port,
	   u_int8_t dst_mac[6], IpAddress *_dst_ip, u_int16_t _dst_port) : GenericHashEntry(_iface) {
  time_t last_recvd = iface->getTimeLastPktRcvd();
  Flow(_iface, _vlanId, _protocol, 
       src_mac, _src_ip, _src_port,
       dst_mac, _dst_ip, _dst_port,
       last_recvd, last_recvd);
}

/* *************************************** */

void Flow::allocFlowMemory() {
  if((ndpi_flow = (ndpi_flow_struct*)calloc(1, iface->get_flow_size())) == NULL)
    throw "Not enough memory";  
  
  if((src_id = calloc(1, iface->get_size_id())) == NULL)
    throw "Not enough memory";  
  
  if((dst_id = calloc(1, iface->get_size_id())) == NULL)
    throw "Not enough memory";  
}

/* *************************************** */

void Flow::deleteFlowMemory() {
  if(ndpi_flow) { free(ndpi_flow); ndpi_flow = NULL; }
  if(src_id)    { free(src_id);    src_id = NULL;    }
  if(dst_id)    { free(dst_id);    dst_id = NULL;    }
}

/* *************************************** */

Flow::~Flow() {
  if(src_host) src_host->decUses();
  if(dst_host) dst_host->decUses();
  if(categorization.category != NULL) free(categorization.category);
  if(json_info) free(json_info);

  deleteFlowMemory();
}

/* *************************************** */

void Flow::setDetectedProtocol(u_int16_t proto_id, u_int8_t l4_proto) {
  if(ndpi_flow != NULL) {
    if(proto_id != NDPI_PROTOCOL_UNKNOWN) {
      u_int16_t sport = htons(src_port), dport = htons(dst_port);

      detected_protocol = proto_id;
    
      switch(detected_protocol) {
      case NDPI_PROTOCOL_DNS:
	if(ntop->getPrefs()->decode_dns_responses()) {
	  if(ndpi_flow->host_server_name[0] != '\0') {
	    char delimiter = '@';
	    char *at = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter);
	    
	    if(at) {
	      at[0] = '\0';
	      ntop->getRedis()->setResolvedAddress(&at[1], (char*)ndpi_flow->host_server_name);
	    }
	  }
	}
	break;

      case NDPI_PROTOCOL_SSL:
	/* 
	   In case of SSL there are probably sub-protocols
	   such as IMAPS that can be otherwise detected
	*/
	if((sport == 465) || (dport == 465)) detected_protocol = NDPI_PROTOCOL_MAIL_SMTP;
	else if((sport == 993) || (dport == 993)) detected_protocol = NDPI_PROTOCOL_MAIL_IMAP;
	else if((sport == 995) || (dport == 995)) detected_protocol = NDPI_PROTOCOL_MAIL_POP;
	/* No break !!!! */

      case NDPI_PROTOCOL_HTTP:
	if(ndpi_flow->host_server_name[0] != '\0') {
	  char buf[64], *doublecol, delimiter = ':';	  
	  Host *svr = (sport < dport) ? src_host : dst_host;

	  /* if <host>:<port> We need to remove ':' */
	  if((doublecol = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter)) != NULL)
	    doublecol[0] = '\0';	  

	  if(svr) {
	    svr->setName((char*)ndpi_flow->host_server_name, true);
	    if(ntop->getRedis()->getFlowCategory((char*)ndpi_flow->host_server_name, buf, sizeof(buf), true) != NULL) {
	      categorization.flow_categorized = true;
	      categorization.category = strdup(buf);
	    }
	    ntop->getRedis()->setResolvedAddress(svr->get_ip()->print(buf, sizeof(buf)),
						 (char*)ndpi_flow->host_server_name);
	  }
	}
	break;
      } /* switch */
      detection_completed = true;
    } else if((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS) {
      detection_completed = true; /* We give up */

      /* We can guess the protocol */
      detected_protocol = ndpi_guess_undetected_protocol(iface->get_ndpi_struct(), protocol,
							 ntohl(src_host->get_ip()->get_ipv4()), ntohs(src_port),
							 ntohl(dst_host->get_ip()->get_ipv4()), ntohs(dst_port));
    }

    if(detection_completed) deleteFlowMemory();
  }
}

/* *************************************** */

void Flow::setJSONInfo(char *json) {
  if (json_info != NULL) free(json_info);
  json_info = strdup(json);
}

/* *************************************** */

int Flow::compare(Flow *fb) {
  int c;

  if(vlanId < fb->vlanId) return(-1); else { if(vlanId > fb->vlanId) return(1); }
  c = src_host->compare(fb->get_src_host()); if(c < 0) return(-1); else { if(c > 0) return(1); }
  if(src_port < fb->src_port) return(-1); else { if(src_port > fb->src_port) return(1); }
  c = dst_host->compare(fb->get_dst_host()); if(c < 0) return(-1); else { if(c > 0) return(1); }
  if(dst_port < fb->dst_port) return(-1); else { if(dst_port > fb->dst_port) return(1); }
  if(protocol < fb->protocol) return(-1); else { if(protocol > fb->protocol) return(1); }

  return(0);
}

/* *************************************** */

char* Flow::ipProto2Name(u_short proto_id) {
  static char proto[8];

  switch(proto_id) {
  case IPPROTO_TCP:
    return((char*)"TCP");
    break;
  case IPPROTO_UDP:
    return((char*)"UDP");
    break;
  case IPPROTO_ICMP:
    return((char*)"ICMP");
    break;
  case 112:
    return((char*)"VRRP");
    break;
  }

  snprintf(proto, sizeof(proto), "%u", proto_id);
  return(proto);
}

/* *************************************** */

/*
 * A faster replacement for inet_ntoa().
 */
char* Flow::intoaV4(unsigned int addr, char* buf, u_short bufLen) {
  char *cp, *retStr;
  uint byte;
  int n;

  cp = &buf[bufLen];
  *--cp = '\0';

  n = 4;
  do {
    byte = addr & 0xff;
    *--cp = byte % 10 + '0';
    byte /= 10;
    if (byte > 0) {
      *--cp = byte % 10 + '0';
      byte /= 10;
      if (byte > 0)
	*--cp = byte + '0';
    }
    *--cp = '.';
    addr >>= 8;
  } while (--n > 0);

  /* Convert the string to srccase */
  retStr = (char*)(cp+1);

  return(retStr);
}

/* *************************************** */

void Flow::print_peers(lua_State* vm) {
  char buf1[64], buf2[64], buf[256];
  Host *src = get_src_host(), *dst = get_dst_host();

  if((src == NULL) || (dst == NULL)) return;
  
  lua_newtable(vm);

  lua_push_str_table_entry(vm, "client", get_src_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_str_table_entry(vm, "server", get_dst_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_int_table_entry(vm, "sent", cli2srv_bytes);
  lua_push_int_table_entry(vm, "rcvd", srv2cli_bytes);
    
  // Key
  snprintf(buf, sizeof(buf), "%s %s", 
	   src->Host::get_name(buf1, sizeof(buf1), false),
	   dst->Host::get_name(buf2, sizeof(buf2), false));

  /*
  snprintf(buf, sizeof(buf), "%s %s",
           intoaV4(ntohl(get_src_ipv4()), buf1, sizeof(buf1)),
           intoaV4(ntohl(get_dst_ipv4()), buf2, sizeof(buf2)));
  */
  lua_pushstring(vm, buf);
  lua_insert(vm, -2);
  lua_settable(vm, -3);  
}

/* *************************************** */

void Flow::print() {
  char buf1[32], buf2[32];

  printf("\t%s %s:%u > %s:%u [proto: %u/%s][%u/%u pkts][%llu/%llu bytes]\n",
	 ipProto2Name(protocol),
	 src_host->get_ip()->print(buf1, sizeof(buf1)), ntohs(src_port),
	 dst_host->get_ip()->print(buf2, sizeof(buf2)), ntohs(dst_port),
	 detected_protocol,
	 ndpi_get_proto_name(iface->get_ndpi_struct(), detected_protocol),
	 cli2srv_packets, srv2cli_packets,
	 (long long unsigned) cli2srv_bytes, (long long unsigned) srv2cli_bytes);
}

/* *************************************** */

void Flow::update_hosts_stats() {
  /* if(detection_completed) */ {
    u_int64_t sent_packets, sent_bytes, rcvd_packets, rcvd_bytes;
    u_int64_t diff_sent_packets, diff_sent_bytes, diff_rcvd_packets, diff_rcvd_bytes;
    
    sent_packets = cli2srv_packets, sent_bytes = cli2srv_bytes;
    diff_sent_packets = sent_packets - cli2srv_last_packets, diff_sent_bytes = sent_bytes - cli2srv_last_bytes;
    cli2srv_last_packets = sent_packets, cli2srv_last_bytes = sent_bytes;
    
    rcvd_packets = srv2cli_packets, rcvd_bytes = srv2cli_bytes;
    diff_rcvd_packets = rcvd_packets - srv2cli_last_packets, diff_rcvd_bytes = rcvd_bytes - srv2cli_last_bytes;
    srv2cli_last_packets = rcvd_packets, srv2cli_last_bytes = rcvd_bytes;
    
    if(src_host)
      src_host->incStats(protocol,  detected_protocol, diff_sent_packets, diff_sent_bytes, 
			 diff_rcvd_packets, diff_rcvd_bytes);
    if(dst_host) 
      dst_host->incStats(protocol, detected_protocol, diff_rcvd_packets, diff_rcvd_bytes, 
			 diff_sent_packets, diff_sent_bytes);
  }
}

/* *************************************** */

bool Flow::equal(IpAddress *_src_ip, IpAddress *_dst_ip, u_int16_t _src_port, 
		 u_int16_t _dst_port, u_int16_t _vlanId, u_int8_t _protocol,
		 bool *src2dst_direction) {
  if((_vlanId != vlanId) || (_protocol != protocol)) return(false);

  if(src_host && src_host->equal(_src_ip) && dst_host && dst_host->equal(_dst_ip) 
     && (_src_port == src_port) && (_dst_port == dst_port)) {
    *src2dst_direction = true;
    return(true);
  } else if(dst_host && dst_host->equal(_src_ip) && src_host && src_host->equal(_dst_ip) 
	    && (_dst_port == src_port) && (_src_port == dst_port)) {
    *src2dst_direction = false;
    return(true);
  } else
    return(false);
}

/* *************************************** */

void Flow::lua(lua_State* vm, bool detailed_dump) {
  char buf[64];

  lua_newtable(vm);

  if(get_src_host()) {
    lua_push_str_table_entry(vm, "src.host", get_src_host()->get_name(buf, sizeof(buf), false));
    lua_push_str_table_entry(vm, "src.ip", get_src_host()->get_ip()->print(buf, sizeof(buf)));
  } else {
    lua_push_nil_table_entry(vm, "src.host");
    lua_push_nil_table_entry(vm, "src.ip");
  }

  lua_push_int_table_entry(vm, "src.port", ntohs(get_src_port()));

  if(get_dst_host()) {
    lua_push_str_table_entry(vm, "dst.host", get_dst_host()->get_name(buf, sizeof(buf), false)); 
    lua_push_str_table_entry(vm, "dst.ip", get_dst_host()->get_ip()->print(buf, sizeof(buf)));
  } else {
    lua_push_nil_table_entry(vm, "dst.host");
    lua_push_nil_table_entry(vm, "dst.ip");    
  }

  lua_push_int_table_entry(vm, "dst.port", ntohs(get_dst_port()));
  lua_push_int_table_entry(vm, "vlan", get_vlan_id());
  lua_push_str_table_entry(vm, "proto.l4", get_protocol_name());

  if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS) 
     || (detected_protocol != NDPI_PROTOCOL_UNKNOWN))
    lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
  else
    lua_push_str_table_entry(vm, "proto.ndpi", (char*)"(Too Early)");

  lua_push_int_table_entry(vm, "bytes", cli2srv_bytes+srv2cli_bytes);
  lua_push_int_table_entry(vm, "seen.first", get_first_seen());
  lua_push_int_table_entry(vm, "seen.last", get_last_seen());
  lua_push_int_table_entry(vm, "duration", get_duration());
  lua_push_int_table_entry(vm, "cli2srv.bytes", cli2srv_bytes);
  lua_push_int_table_entry(vm, "srv2cli.bytes", srv2cli_bytes);
  lua_push_str_table_entry(vm, "category", categorization.category ? categorization.category : (char*)"");
    
  lua_push_str_table_entry(vm, "moreinfo.json", get_json_info());
  
  if(!detailed_dump) {
    lua_pushinteger(vm, key()); // Index
    lua_insert(vm, -2);
    lua_settable(vm, -3);
  } else {
    lua_push_int_table_entry(vm, "cli2srv.packets", cli2srv_packets);
    lua_push_int_table_entry(vm, "srv2cli.packets", srv2cli_packets);
  }
}

/* *************************************** */

u_int32_t Flow::key() {
  u_int32_t k = src_port+dst_port+vlanId+protocol;
  
  if(src_host) k += src_host->key();
  if(dst_host) k += dst_host->key();

  return(k);
}

/* *************************************** */

bool Flow::idle() {
  return(isIdle(ntop->getPrefs()->get_host_max_idle())); 
};

/* *************************************** */

char* Flow::getDomainCategory() {
  if(!categorization.flow_categorized) {
    if(ndpi_flow == NULL)
      categorization.flow_categorized = true;
    else if(ndpi_flow->host_server_name) {
      if(ntop->getRedis()->getFlowCategory((char*)ndpi_flow->host_server_name,
					   categorization.category, sizeof(categorization.category), 
					   false) != NULL)
	categorization.flow_categorized = true;
    }
  }

  return(categorization.category);
}
