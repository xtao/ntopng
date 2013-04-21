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
	   u_int32_t _src_ip, u_int16_t _src_port,
	   u_int32_t _dst_ip, u_int16_t _dst_port) : HashEntry(_iface) {
  vlanId = _vlanId, protocol = _protocol,
    src_ip = _src_ip, src_port = _src_port,
    dst_ip = _dst_ip, dst_port = _dst_port;
  cli2srv_packets = cli2srv_bytes = srv2cli_packets = srv2cli_bytes = cli2srv_last_packets = cli2srv_last_bytes = srv2cli_last_packets = srv2cli_last_bytes = 0;
  
 detection_completed = false, detected_protocol = NDPI_PROTOCOL_UNKNOWN;
  ndpi_flow = NULL, src_id = dst_id = NULL;

  iface->findFlowHosts(this, &src_host, &dst_host);
  if(src_host) src_host->incUses();
  if(dst_host) dst_host->incUses();
  first_seen = last_seen = iface->getTimeLastPktRcvd();
  allocFlowMemory();
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
  deleteFlowMemory();
}

/* *************************************** */

void Flow::setDetectedProtocol(u_int16_t proto_id, u_int8_t l4_proto) {
  if(proto_id != NDPI_PROTOCOL_UNKNOWN) {
    detected_protocol = proto_id;
    
    if((detected_protocol != NDPI_PROTOCOL_UNKNOWN)
       || (l4_proto == IPPROTO_UDP)
       || ((l4_proto == IPPROTO_TCP) && ((cli2srv_packets+srv2cli_packets) > 10))) {
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

      case NDPI_PROTOCOL_HTTP:
      case NDPI_PROTOCOL_SSL:	
	if(ndpi_flow->host_server_name[0] != '\0') {
	  char buf[64], *doublecol, delimiter = ':';	  
	  Host *svr = (htons(src_port) < htons(dst_port)) ? src_host : dst_host;

	  /* if <host>:<port> We need to remove ':' */
	  if((doublecol = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter)) != NULL)
	     doublecol[0] = '\0';	  

	  svr->setName((char*)ndpi_flow->host_server_name);

	  ntop->getRedis()->setResolvedAddress(svr->get_ip()->print(buf, sizeof(buf)),
					       (char*)ndpi_flow->host_server_name);
	}
	break;
      }

      detection_completed = true;
      deleteFlowMemory();
    }
  }
}

/* *************************************** */

int Flow::compare(Flow *fb) {
  if(vlanId < fb->vlanId) return(-1); else { if(vlanId > fb->vlanId) return(1); }
  if(src_ip < fb->src_ip) return(-1); else { if(src_ip > fb->src_ip) return(1); }
  if(src_port < fb->src_port) return(-1); else { if(src_port > fb->src_port) return(1); }
  if(dst_ip < fb->dst_ip) return(-1); else { if(dst_ip > fb->dst_ip) return(1); }
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
  char buf1[32], buf2[32], buf[256];
  Host *src = get_src_host(), *dst = get_dst_host();

  if((src == NULL) || (dst == NULL)) return;
  
  lua_newtable(vm);

  // Sent
  lua_pushstring(vm, "sent");
  lua_pushnumber(vm, cli2srv_bytes);
  lua_settable(vm, -3);
  
  // Rcvd
  lua_pushstring(vm, "rcvd");
  lua_pushnumber(vm, srv2cli_bytes);
  lua_settable(vm, -3);
    
  // Key
  snprintf(buf, sizeof(buf), "%s %s", 
	   src->Host::get_name(buf1, sizeof(buf1)),
	   dst->Host::get_name(buf2, sizeof(buf2)));

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

  printf("\t%s %s:%u > %s:%u [proto: %u/%s][%u/%u pkts][%u/%u bytes]\n",
	 ipProto2Name(protocol),
	 intoaV4(ntohl(src_ip), buf1, sizeof(buf1)),
	 ntohs(src_port),
	 intoaV4(ntohl(dst_ip), buf2, sizeof(buf2)),
	 ntohs(dst_port),
	 detected_protocol,
	 ndpi_get_proto_name(iface->get_ndpi_struct(), detected_protocol),
	 cli2srv_packets, srv2cli_packets,
	 cli2srv_bytes, srv2cli_bytes);
}

/* *************************************** */

void Flow::update_hosts_stats() {
  if(detection_completed) {
    u_int32_t sent_packets, sent_bytes, rcvd_packets, rcvd_bytes;
    u_int32_t diff_sent_packets, diff_sent_bytes, diff_rcvd_packets, diff_rcvd_bytes;
    
    sent_packets = cli2srv_packets, sent_bytes = cli2srv_bytes;
    diff_sent_packets = sent_packets - cli2srv_last_packets, diff_sent_bytes = sent_bytes - cli2srv_last_bytes;
    cli2srv_last_packets = sent_packets, cli2srv_last_bytes = sent_bytes;
    
    rcvd_packets = srv2cli_packets, rcvd_bytes = srv2cli_bytes;
    diff_rcvd_packets = rcvd_packets - srv2cli_last_packets, diff_rcvd_bytes = rcvd_bytes - srv2cli_last_bytes;
    srv2cli_last_packets = rcvd_packets, srv2cli_last_bytes = rcvd_bytes;
    
    if(src_host)
      src_host->incStats(detected_protocol, diff_sent_packets, diff_sent_bytes, 
			 diff_rcvd_packets, diff_rcvd_bytes);
    if(dst_host) 
      dst_host->incStats(detected_protocol, diff_rcvd_packets, diff_rcvd_bytes, 
			 diff_sent_packets, diff_sent_bytes);
  }
}

/* *************************************** */

bool Flow::equal(u_int32_t _src_ip, u_int32_t _dst_ip, u_int16_t _src_port, u_int16_t _dst_port, u_int16_t _vlanId, u_int8_t _protocol) {
  if((_vlanId != vlanId) || (_protocol != protocol)) return(false);

  if(((_src_ip == src_ip) || (_dst_ip == dst_ip) || (_src_port == src_port) || (_dst_port == dst_port))
     || ((_dst_ip == src_ip) || (_src_ip == dst_ip) || (_dst_port == src_port) || (_src_port == dst_port)))
    return(true);
  else
    return(false);
}


/* *************************************** */

void Flow::dumpFlowToLua(lua_State* vm, bool detailed_dump) {
  char buf[64];

  lua_newtable(vm);
  lua_push_str_table_entry(vm, "src.host", get_src_host()->get_name(buf, sizeof(buf)));
  lua_push_str_table_entry(vm, "src.ip", get_src_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_int_table_entry(vm, "src.port", get_src_port());
  lua_push_str_table_entry(vm, "dst.host", get_dst_host()->get_name(buf, sizeof(buf)));
  lua_push_str_table_entry(vm, "dst.ip", get_dst_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_int_table_entry(vm, "dst.port", get_dst_port());
  lua_push_int_table_entry(vm, "vlan", get_vlan_id());
  lua_push_str_table_entry(vm, "proto.l4", get_protocol_name());
  lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
  lua_push_int_table_entry(vm, "bytes", cli2srv_bytes+srv2cli_bytes);

  lua_pushinteger(vm, key());
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

