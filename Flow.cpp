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
	   u_int8_t cli_mac[6], IpAddress *_cli_ip, u_int16_t _cli_port,
	   u_int8_t srv_mac[6], IpAddress *_srv_ip, u_int16_t _srv_port,
	   time_t _first_seen, time_t _last_seen) : GenericHashEntry(_iface) {
  vlanId = _vlanId, protocol = _protocol, cli_port = _cli_port, srv_port = _srv_port;
  cli2srv_packets = 0, cli2srv_bytes = 0, srv2cli_packets = 0, srv2cli_bytes = 0, cli2srv_last_packets = 0,
    cli2srv_last_bytes = 0, srv2cli_last_packets = 0, srv2cli_last_bytes = 0;

  detection_completed = false, detected_protocol = NDPI_PROTOCOL_UNKNOWN;
  ndpi_flow = NULL, cli_id = srv_id = NULL;
  json_info = strdup("{}");
  tcp_flags = 0, last_update_time.tv_sec = 0, bytes_thpt = 0;
  cli2srv_last_bytes = prev_cli2srv_last_bytes = 0, srv2cli_last_bytes = prev_srv2cli_last_bytes = 0;

  iface->findFlowHosts(_vlanId, cli_mac, _cli_ip, &cli_host, srv_mac, _srv_ip, &srv_host);
  if(cli_host) { cli_host->incUses(); if(srv_host) cli_host->incrContact(srv_host, true);  }
  if(srv_host) { srv_host->incUses(); if(cli_host) srv_host->incrContact(cli_host, false); }
  first_seen = _first_seen, last_seen = _last_seen;
  categorization.category = NULL, categorization.flow_categorized = false;
  bytes_thpt_trend = trend_unknown;
  protocol_processed = false;
  /*
    NOTE

    We enable nDPI even if this is a flow collector interface
    where DPI cannot be used. This is because we will use nDPI
    to guess protocols based on ports and IPs
   */
  /* if(iface->is_ndpi_enabled()) */ allocFlowMemory();
}

/* *************************************** */

Flow::Flow(NetworkInterface *_iface,
	   u_int16_t _vlanId, u_int8_t _protocol,
	   u_int8_t cli_mac[6], IpAddress *_cli_ip, u_int16_t _cli_port,
	   u_int8_t srv_mac[6], IpAddress *_srv_ip, u_int16_t _srv_port) : GenericHashEntry(_iface) {
  time_t last_recvd = iface->getTimeLastPktRcvd();

  Flow(_iface, _vlanId, _protocol, cli_mac, _cli_ip, _cli_port,
       srv_mac, _srv_ip, _srv_port, last_recvd, last_recvd);
}

/* *************************************** */

void Flow::allocFlowMemory() {
  if((ndpi_flow = (ndpi_flow_struct*)calloc(1, iface->get_flow_size())) == NULL)
    throw "Not enough memory";

  if((cli_id = calloc(1, iface->get_size_id())) == NULL)
    throw "Not enough memory";

  if((srv_id = calloc(1, iface->get_size_id())) == NULL)
    throw "Not enough memory";
}

/* *************************************** */

void Flow::deleteFlowMemory() {
  if(ndpi_flow) { free(ndpi_flow); ndpi_flow = NULL; }
  if(cli_id)    { free(cli_id);    cli_id = NULL;    }
  if(srv_id)    { free(srv_id);    srv_id = NULL;    }
}

/* *************************************** */

Flow::~Flow() {
#ifdef HAVE_SQLITE
  DB *db = ntop->get_db();

  if(db) db->dumpFlow(last_seen, this);
#endif

  if(cli_host) cli_host->decUses();
  if(srv_host) srv_host->decUses();
  if(categorization.category != NULL) free(categorization.category);
  if(json_info) free(json_info);

  deleteFlowMemory();
}

/* *************************************** */

void Flow::aggregateInfo(char *name, u_int8_t l4_proto, u_int16_t ndpi_proto_id) {
  if(ntop->getPrefs()->get_aggregation_mode() != aggregations_disabled) {
    StringHost *host = iface->findHostByString(name, ndpi_proto_id, true);
    
    if(host != NULL) {
      host->incStats(l4_proto, ndpi_proto_id, 0, 0, 1, 1 /* Dummy */);
      host->updateSeen();
      host->updateActivities();
      if(cli_host) host->incrContact(iface, cli_host->get_ip(), true);
    }
  }
}

/* *************************************** */

void Flow::processDetectedProtocol() {
  if(protocol_processed || (ndpi_flow == NULL)) return;

  switch(detected_protocol) {
  case NDPI_PROTOCOL_DNS:
    if(ntop->getPrefs()->decode_dns_responses()) {
      if(ndpi_flow->host_server_name[0] != '\0') {
	char delimiter = '@', *name = NULL;
	char *at = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter);
	
	if(at != NULL)
	  name = &at[1], at[0] = '\0';
	else if(!strstr((const char*)ndpi_flow->host_server_name, ".in-addr.arpa"))
	  name = (char*)ndpi_flow->host_server_name;

	if(name) {
	  protocol_processed = true;

	  if(ndpi_flow->protos.dns.num_answer_rrs > 0)
	    ntop->getRedis()->setResolvedAddress(name, (char*)ndpi_flow->host_server_name);

	  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[DNS] %s", (char*)ndpi_flow->host_server_name);

	  aggregateInfo((char*)ndpi_flow->host_server_name, protocol, detected_protocol);
	}
      }
    }
    break;

  case NDPI_PROTOCOL_WHOIS_DAS:
    if(ndpi_flow->host_server_name[0] != '\0') {
      protocol_processed = true;

      for(int i=0; ndpi_flow->host_server_name[i] != '\0'; i++)
	ndpi_flow->host_server_name[i] = tolower(ndpi_flow->host_server_name[i]);

      aggregateInfo((char*)ndpi_flow->host_server_name, protocol, detected_protocol);
    }
    break;

  case NDPI_PROTOCOL_SSL:
  case NDPI_PROTOCOL_HTTP:
    if(ndpi_flow->host_server_name[0] != '\0') {
      char buf[64], *doublecol, delimiter = ':';
      u_int16_t sport = htons(cli_port), dport = htons(srv_port);
      Host *svr = (sport < dport) ? cli_host : srv_host;

      protocol_processed = true;

      /* if <host>:<port> We need to remove ':' */
      if((doublecol = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter)) != NULL)
	doublecol[0] = '\0';

      if(svr) {
	svr->setName((char*)ndpi_flow->host_server_name, true);
	aggregateInfo((char*)ndpi_flow->host_server_name, protocol, detected_protocol);

	if(ntop->getRedis()->getFlowCategory((char*)ndpi_flow->host_server_name, 
					     buf, sizeof(buf), true) != NULL) {
	  categorization.flow_categorized = true;
	  categorization.category = strdup(buf);
	}
	ntop->getRedis()->setResolvedAddress(svr->get_ip()->print(buf, sizeof(buf)),
					     (char*)ndpi_flow->host_server_name);

	if(ndpi_flow->detected_os[0] != '\0') {
	  aggregateInfo((char*)ndpi_flow->detected_os, protocol, NTOPNG_NDPI_OS_PROTO_ID);
	  if(cli_host)
	    cli_host->setOS((char*)ndpi_flow->detected_os);
	}
      }
    }
    break;
  } /* switch */

  if(protocol_processed)
    deleteFlowMemory();
}

/* *************************************** */

void Flow::setDetectedProtocol(u_int16_t proto_id) {
  if((ndpi_flow != NULL) || (!iface->is_ndpi_enabled())) {
    if(proto_id != NDPI_PROTOCOL_UNKNOWN) {
      detected_protocol = proto_id;
      processDetectedProtocol();
      detection_completed = true;
    } else if((((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
	       && (cli_host != NULL)
	       && (srv_host != NULL))
	      || (!iface->is_ndpi_enabled())) {
      detection_completed = true; /* We give up */

      /* We can guess the protocol */
      detected_protocol = ndpi_guess_undetected_protocol(iface->get_ndpi_struct(), protocol,
							 ntohl(cli_host->get_ip()->get_ipv4()), ntohs(cli_port),
							 ntohl(srv_host->get_ip()->get_ipv4()), ntohs(srv_port));
    }

    //if(detection_completed) deleteFlowMemory();
  }
}

/* *************************************** */

void Flow::setJSONInfo(const char *json) {
  if(json == NULL) return;

  if (json_info != NULL) free(json_info);
  json_info = strdup(json);
}

/* *************************************** */

int Flow::compare(Flow *fb) {
  int c;

  if((cli_host == NULL) || (srv_host == NULL)) return(-1);

  if(vlanId < fb->vlanId) return(-1); else { if(vlanId > fb->vlanId) return(1); }
  c = cli_host->compare(fb->get_cli_host()); if(c < 0) return(-1); else { if(c > 0) return(1); }
  if(cli_port < fb->cli_port) return(-1); else { if(cli_port > fb->cli_port) return(1); }
  c = srv_host->compare(fb->get_srv_host()); if(c < 0) return(-1); else { if(c > 0) return(1); }
  if(srv_port < fb->srv_port) return(-1); else { if(srv_port > fb->srv_port) return(1); }
  if(protocol < fb->protocol) return(-1); else { if(protocol > fb->protocol) return(1); }

  return(0);
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

u_int64_t Flow::get_current_bytes_cli2srv() {
  if((cli2srv_last_bytes == 0) && (prev_cli2srv_last_bytes == 0))
    return(cli2srv_bytes);
  else {
    int64_t diff = cli2srv_bytes - cli2srv_last_bytes;

    if(diff > 0)
      return(diff);

    /*
       We need to do this as due to concurrency issues,
       we might have a negative value
    */
    diff = cli2srv_bytes - prev_cli2srv_last_bytes;

    if(diff > 0)
      return(diff);
    else
      return(0);
  }
};

/* *************************************** */

u_int64_t Flow::get_current_bytes_srv2cli() {
  if((srv2cli_last_bytes == 0) && (prev_srv2cli_last_bytes == 0))
    return(srv2cli_bytes);
  else {
    int64_t diff = srv2cli_bytes - srv2cli_last_bytes;

    if(diff > 0)
      return(diff);

    /*
       We need to do this as due to concurrency issues,
       we might have a negative value
    */
    diff = srv2cli_bytes - prev_srv2cli_last_bytes;

    if(diff > 0)
      return(diff);
    else
      return(0);
  }
};

/* *************************************** */

void Flow::print_peers(lua_State* vm, bool verbose) {
  char buf1[64], buf2[64], buf[256];
  Host *src = get_cli_host(), *dst = get_srv_host();

  if((src == NULL) || (dst == NULL)) return;

  lua_newtable(vm);

  lua_push_str_table_entry(vm,  "client", get_cli_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_str_table_entry(vm,  "server", get_srv_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_int_table_entry(vm,  "sent", cli2srv_bytes);
  lua_push_int_table_entry(vm,  "rcvd", srv2cli_bytes);
  lua_push_int_table_entry(vm,  "sent.last", get_current_bytes_cli2srv());
  lua_push_int_table_entry(vm,  "rcvd.last", get_current_bytes_srv2cli());
  lua_push_int_table_entry(vm,  "duration", get_duration());

  if(verbose) {
    lua_push_bool_table_entry(vm, "client.private", get_cli_host()->get_ip()->isPrivateAddress());
    lua_push_str_table_entry(vm,  "client.country", get_cli_host()->get_country() ? get_cli_host()->get_country() : (char*)"");
    lua_push_bool_table_entry(vm, "server.private", get_srv_host()->get_ip()->isPrivateAddress());
    lua_push_str_table_entry(vm,  "server.country", get_srv_host()->get_country() ? get_srv_host()->get_country() : (char*)"");
    lua_push_float_table_entry(vm, "client.latitude", get_cli_host()->get_latitude());
    lua_push_float_table_entry(vm, "client.longitude", get_cli_host()->get_longitude());
    lua_push_str_table_entry(vm, "client.city", get_cli_host()->get_city() ? get_cli_host()->get_city() : (char*)"");
    lua_push_float_table_entry(vm, "server.latitude", get_srv_host()->get_latitude());
    lua_push_float_table_entry(vm, "server.longitude", get_srv_host()->get_longitude());
    lua_push_str_table_entry(vm, "server.city", get_srv_host()->get_city() ? get_srv_host()->get_city() : (char*)"");

    if(verbose) {
      if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
	 || (detected_protocol != NDPI_PROTOCOL_UNKNOWN)
	 || iface->is_ndpi_enabled())
	lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
      else
	lua_push_str_table_entry(vm, "proto.ndpi", (char*)"(Too Early)");
    }
  }

  // Key
  /* Too slow */
#if 0
  snprintf(buf, sizeof(buf), "%s %s",
	   src->Host::get_name(buf1, sizeof(buf1), false),
	   dst->Host::get_name(buf2, sizeof(buf2), false));
#else
  snprintf(buf, sizeof(buf), "%s %s",
           intoaV4(ntohl(get_cli_ipv4()), buf1, sizeof(buf1)),
           intoaV4(ntohl(get_srv_ipv4()), buf2, sizeof(buf2)));
#endif

  lua_pushstring(vm, buf);
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

void Flow::print() {
  char buf1[32], buf2[32];

  if((cli_host == NULL) || (srv_host == NULL)) return;

  printf("\t%s %s:%u > %s:%u [proto: %u/%s][%u/%u pkts][%llu/%llu bytes]\n",
	 get_protocol_name(),
	 cli_host->get_ip()->print(buf1, sizeof(buf1)), ntohs(cli_port),
	 srv_host->get_ip()->print(buf2, sizeof(buf2)), ntohs(srv_port),
	 detected_protocol,
	 ndpi_get_proto_name(iface->get_ndpi_struct(), detected_protocol),
	 cli2srv_packets, srv2cli_packets,
	 (long long unsigned) cli2srv_bytes, (long long unsigned) srv2cli_bytes);
}

/* *************************************** */

void Flow::update_hosts_stats(struct timeval *tv) {
  u_int64_t sent_packets, sent_bytes, rcvd_packets, rcvd_bytes;
  u_int64_t diff_sent_packets, diff_sent_bytes, diff_rcvd_packets, diff_rcvd_bytes;

  sent_packets = cli2srv_packets, sent_bytes = cli2srv_bytes;
  diff_sent_packets = sent_packets - cli2srv_last_packets, diff_sent_bytes = sent_bytes - cli2srv_last_bytes;
  prev_cli2srv_last_bytes = cli2srv_last_bytes;
  cli2srv_last_packets = sent_packets, cli2srv_last_bytes = sent_bytes;

  rcvd_packets = srv2cli_packets, rcvd_bytes = srv2cli_bytes;
  diff_rcvd_packets = rcvd_packets - srv2cli_last_packets, diff_rcvd_bytes = rcvd_bytes - srv2cli_last_bytes;
  prev_srv2cli_last_bytes = srv2cli_last_bytes;
  srv2cli_last_packets = rcvd_packets, srv2cli_last_bytes = rcvd_bytes;

  if(cli_host)
    cli_host->incStats(protocol,  detected_protocol, diff_sent_packets, diff_sent_bytes,
		       diff_rcvd_packets, diff_rcvd_bytes);
  if(srv_host)
    srv_host->incStats(protocol, detected_protocol, diff_rcvd_packets, diff_rcvd_bytes,
		       diff_sent_packets, diff_sent_bytes);

  if(last_update_time.tv_sec > 0) {
    float tdiff_msec = ((float)(tv->tv_sec-last_update_time.tv_sec)*1000)+((tv->tv_usec-last_update_time.tv_usec)/(float)1000);
    float bytes_msec = ((float)((cli2srv_last_bytes-prev_cli2srv_last_bytes)*1000))/tdiff_msec;

    if(bytes_msec < 0) bytes_msec = 0; /* Just to be safe */

    if(bytes_thpt < bytes_msec)      bytes_thpt_trend = trend_up;
    else if(bytes_thpt > bytes_msec) bytes_thpt_trend = trend_down;
    else                             bytes_thpt_trend = trend_stable;

    /*
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "[msec: %.1f][bytes: %u][bits_thpt: %.2f Mbps]",
				 bytes_msec, (cli2srv_last_bytes-prev_cli2srv_last_bytes),
				 (bytes_thpt*8)/((float)(1024*1024)));
    */
    bytes_thpt = bytes_msec;
  }

  memcpy(&last_update_time, tv, sizeof(struct timeval));
}

/* *************************************** */

bool Flow::equal(IpAddress *_cli_ip, IpAddress *_srv_ip, u_int16_t _cli_port,
		 u_int16_t _srv_port, u_int16_t _vlanId, u_int8_t _protocol,
		 bool *src2srv_direction) {
  if((_vlanId != vlanId) || (_protocol != protocol)) return(false);

  if(cli_host && cli_host->equal(_cli_ip) && srv_host && srv_host->equal(_srv_ip)
     && (_cli_port == cli_port) && (_srv_port == srv_port)) {
    *src2srv_direction = true;
    return(true);
  } else if(srv_host && srv_host->equal(_cli_ip) && cli_host && cli_host->equal(_srv_ip)
	    && (_srv_port == cli_port) && (_cli_port == srv_port)) {
    *src2srv_direction = false;
    return(true);
  } else
    return(false);
}

/* *************************************** */

void Flow::lua(lua_State* vm, bool detailed_dump) {
  char buf[64];

  lua_newtable(vm);

  if(get_cli_host()) {
    if(detailed_dump) lua_push_str_table_entry(vm, "cli.host", get_cli_host()->get_name(buf, sizeof(buf), false));
    lua_push_str_table_entry(vm, "cli.ip", get_cli_host()->get_ip()->print(buf, sizeof(buf)));
  } else {
    lua_push_nil_table_entry(vm, "cli.host");
    lua_push_nil_table_entry(vm, "cli.ip");
  }

  lua_push_int_table_entry(vm, "cli.port", get_cli_port());

  if(get_srv_host()) {
    if(detailed_dump) lua_push_str_table_entry(vm, "srv.host", get_srv_host()->get_name(buf, sizeof(buf), false));
    lua_push_str_table_entry(vm, "srv.ip", get_srv_host()->get_ip()->print(buf, sizeof(buf)));
  } else {
    lua_push_nil_table_entry(vm, "srv.host");
    lua_push_nil_table_entry(vm, "srv.ip");
  }

  lua_push_int_table_entry(vm, "srv.port", get_srv_port());
  lua_push_int_table_entry(vm, "vlan", get_vlan_id());
  lua_push_str_table_entry(vm, "proto.l4", get_protocol_name());

  if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
     || (detected_protocol != NDPI_PROTOCOL_UNKNOWN)
     || iface->is_ndpi_enabled())
    lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
  else
    lua_push_str_table_entry(vm, "proto.ndpi", (char*)CONST_TOO_EARLY);

  lua_push_int_table_entry(vm, "bytes", cli2srv_bytes+srv2cli_bytes);
  lua_push_int_table_entry(vm, "bytes.last", get_current_bytes_cli2srv() + get_current_bytes_srv2cli());
  lua_push_int_table_entry(vm, "seen.first", get_first_seen());
  lua_push_int_table_entry(vm, "seen.last", get_last_seen());
  lua_push_int_table_entry(vm, "duration", get_duration());

  lua_push_int_table_entry(vm, "cli2srv.bytes", cli2srv_bytes);
  lua_push_int_table_entry(vm, "srv2cli.bytes", srv2cli_bytes);

  if(detailed_dump) {
    lua_push_int_table_entry(vm, "tcp_flags", getTcpFlags());
    lua_push_str_table_entry(vm, "category", categorization.category ? categorization.category : (char*)"");
    lua_push_str_table_entry(vm, "moreinfo.json", get_json_info());
  }

  //ntop->getTrace()->traceEvent(TRACE_NORMAL, "%.2f", bytes_thpt);
  lua_push_float_table_entry(vm, "throughput", bytes_thpt);
  lua_push_int_table_entry(vm, "throughput_trend", bytes_thpt_trend);

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
  u_int32_t k = cli_port+srv_port+vlanId+protocol;

  if(cli_host) k += cli_host->key();
  if(srv_host) k += srv_host->key();

  return(k);
}

/* *************************************** */

bool Flow::idle() {
  /* If this flow is idle for at least MAX_TCP_FLOW_IDLE */
  if((protocol == IPPROTO_TCP)
     && ((tcp_flags & TH_FIN) || (tcp_flags & TH_RST))
     && isIdle(MAX_TCP_FLOW_IDLE /* sec */)) {
    /* ntop->getTrace()->traceEvent(TRACE_NORMAL, "[TCP] Early flow expire"); */
    return(true);
  }

  return(isIdle(ntop->getPrefs()->get_host_max_idle()));
};

/* *************************************** */

bool Flow::isFlowPeer(char *numIP) {
  char s_buf[32], *ret;

  ret = cli_host->get_ip()->print(s_buf, sizeof(s_buf));
  if(strcmp(ret, numIP) == 0) return(true);

  ret = srv_host->get_ip()->print(s_buf, sizeof(s_buf));
  if(strcmp(ret, numIP) == 0) return(true);

  return(false);
}

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

/* *************************************** */

void Flow::sumStats(NdpiStats *stats) {
  stats-> incStats(detected_protocol,
		   cli2srv_packets, cli2srv_bytes,
		   srv2cli_packets, srv2cli_bytes);
}

/* *************************************** */

char* Flow::serialize() {
  json_object *my_object, *inner;
  char *rsp, buf[64];

  my_object = json_object_new_object();

  inner = json_object_new_object();
  json_object_object_add(inner, "ip",   json_object_new_string(cli_host->get_string_key(buf, sizeof(buf))));
  json_object_object_add(inner, "port", json_object_new_int(get_cli_port()));
  json_object_object_add(my_object, "client", inner);

  inner = json_object_new_object();
  json_object_object_add(inner, "ip",   json_object_new_string(srv_host->get_string_key(buf, sizeof(buf))));
  json_object_object_add(inner, "port", json_object_new_int(get_srv_port()));
  json_object_object_add(my_object, "server", inner);

  inner = json_object_new_object();
  json_object_object_add(inner, "first", json_object_new_int((u_int32_t)first_seen));
  json_object_object_add(inner, "last", json_object_new_int((u_int32_t)last_seen));
  json_object_object_add(my_object, "seen", inner);

  if(vlanId > 0) json_object_object_add(my_object, "vlanId", json_object_new_int(vlanId));

  json_object_object_add(my_object, "throughput", json_object_new_double(bytes_thpt));
  json_object_object_add(my_object, "throughput_trend", json_object_new_string(Utils::trend2str(bytes_thpt_trend)));

  inner = json_object_new_object();
  json_object_object_add(inner, "l4", json_object_new_int(protocol));
  if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
     || (detected_protocol != NDPI_PROTOCOL_UNKNOWN))
    json_object_object_add(inner, "ndpi", json_object_new_string(get_detected_protocol_name()));
  json_object_object_add(my_object, "proto", inner);

  if(protocol == IPPROTO_TCP) {
    inner = json_object_new_object();
    json_object_object_add(inner, "flags", json_object_new_int(tcp_flags));
    json_object_object_add(my_object, "tcp", inner);
  }

  if(json_info && strcmp(json_info, "{}")) json_object_object_add(my_object, "json", json_object_new_string(json_info));
  if(categorization.flow_categorized) json_object_object_add(my_object, "category", json_object_new_string(categorization.category));

  inner = json_object_new_object();
  json_object_object_add(inner, "packets", json_object_new_int64(cli2srv_packets));
  json_object_object_add(inner, "bytes", json_object_new_int64(cli2srv_bytes));
  json_object_object_add(my_object, "cli2srv", inner);

  inner = json_object_new_object();
  json_object_object_add(inner, "packets", json_object_new_int64(srv2cli_packets));
  json_object_object_add(inner, "bytes", json_object_new_int64(srv2cli_bytes));
  json_object_object_add(my_object, "srv2cli", inner);

  /* JSON string */
  rsp = strdup(json_object_to_json_string(my_object));

  /* Free memory */
  json_object_put(my_object);

  return(rsp);
}

/* *************************************** */

void Flow::incStats(bool cli2srv_direction, u_int pkt_len) {
  updateSeen();

  if((cli_host == NULL) || (srv_host == NULL)) return;

  if(cli2srv_direction) {
    cli2srv_packets++, cli2srv_bytes += pkt_len;
    cli_host->get_sent_stats()->incStats(pkt_len), srv_host->get_recv_stats()->incStats(pkt_len);
  } else {
    srv2cli_packets++, srv2cli_bytes += pkt_len;
    cli_host->get_recv_stats()->incStats(pkt_len), srv_host->get_sent_stats()->incStats(pkt_len);
  }
};


/* *************************************** */

void Flow::updateActivities() {
  if(cli_host) cli_host->updateActivities();
  if(srv_host) srv_host->updateActivities();
}

/* *************************************** */

void Flow::addFlowStats(bool cli2srv_direction, u_int in_pkts, u_int in_bytes,
			u_int out_pkts, u_int out_bytes, time_t last_seen) {
  updateSeen(last_seen); 

  if (cli2srv_direction) 
    cli2srv_packets += in_pkts, cli2srv_bytes += in_bytes, srv2cli_packets += out_pkts, srv2cli_bytes += out_bytes;
  else
    cli2srv_packets += out_pkts, cli2srv_bytes += out_bytes, srv2cli_packets += in_pkts, srv2cli_bytes += in_bytes; 

  updateActivities();
}
