/*
 *
 * (C) 2013-14 - ntop.org
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

  detection_completed = false, ndpi_detected_protocol = NDPI_PROTOCOL_UNKNOWN;
  ndpi_flow = NULL, cli_id = srv_id = NULL, client_proc = server_proc = NULL;
  json_info = strdup("{}");
  tcp_flags = 0, last_update_time.tv_sec = 0, bytes_thpt = 0;
  cli2srv_last_bytes = prev_cli2srv_last_bytes = 0, srv2cli_last_bytes = prev_srv2cli_last_bytes = 0;
  cli2srv_last_packets = prev_cli2srv_last_packets = 0, srv2cli_last_packets = prev_srv2cli_last_packets = 0;

  iface->findFlowHosts(_vlanId, cli_mac, _cli_ip, &cli_host, srv_mac, _srv_ip, &srv_host);
  if(cli_host) { cli_host->incUses(); if(srv_host) cli_host->incrContact(srv_host, true);  }
  if(srv_host) { srv_host->incUses(); if(cli_host) srv_host->incrContact(cli_host, false); }
  first_seen = _first_seen, last_seen = _last_seen;
  categorization.category = NULL, categorization.flow_categorized = false;
  bytes_thpt_trend = trend_unknown;
  pkts_thpt_trend = trend_unknown;
  protocol_processed = false;

  aggregationInfo.name = NULL;
  /*
    NOTE

    We enable nDPI even if this is a flow collector interface
    where DPI cannot be used. This is because we will use nDPI
    to guess protocols based on ports and IPs
   */
  /* if(iface->is_ndpi_enabled()) */ allocFlowMemory();

  // refresh_process();
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
  if(ntop->getPrefs()->do_dump_flows_on_db() 
     || ntop->get_export_interface()) {
    char *json;

    json = serialize();
    cli_host->getInterface()->dumpFlow(last_seen, this, json);  
    
    if(ntop->get_export_interface())
      ntop->get_export_interface()->export_data(json);

    if(json) free(json);
  }

  if(cli_host) cli_host->decUses();
  if(srv_host) srv_host->decUses();
  if(categorization.category != NULL) free(categorization.category);
  if(json_info) free(json_info);
  if(client_proc) delete(client_proc);
  if(server_proc) delete(server_proc);

  if(aggregationInfo.name) free(aggregationInfo.name);
  deleteFlowMemory();
}

/* *************************************** */

void Flow::aggregateInfo(char *_name, u_int16_t ndpi_proto_id,
			 AggregationType mode,
			 bool aggregation_to_track
			 /*
			    i.e. it is not here for an error (such as NXDOMAIN)
			    so we do not store persistently on disk aggregations
			    due to errors that might fill-up the disk quickly
			 */) {
  if((_name == NULL) || (_name[0] == '\0'))
    return; /* Nothing to do */

  if(ntop->getPrefs()->get_aggregation_mode() != aggregations_disabled) {
    StringHost *host;
    char *name = _name, *first_name = NULL;

    if(ntop->getPrefs()->use_short_aggregation_names()
       && (mode == aggregation_domain_name)
       && (strlen(name) > 3 /* .XX */)) {
      u_int num = 0, i;

    /* In order to reduce the number of hosts we can shorten
       the host name and limit it to two levels in domain name
    */

      for(i=strlen(_name)-2; i>0; i--) {
	if(_name[i] == '.') {
	  num++;

	  first_name = &_name[i+1];

	  if(num == 2)
	    name = &_name[i+1];
	  else if(num > 2) {
	    name = first_name;
	  }
	}
      }
    }

#if 0
    if(mode == aggregation_domain_name)
      ntop->getTrace()->traceEvent(TRACE_WARNING, "%s", name);
    else
      return; // FIX
#endif

    host = iface->findHostByString(name, ndpi_proto_id, true);

    if(host != NULL) {
      host->set_aggregation_mode(mode);

      if(aggregationInfo.name && strcmp(aggregationInfo.name, name)) {
	struct timeval tv;

	tv.tv_sec = (long)iface->getTimeLastPktRcvd(), tv.tv_usec = 0;
	update_hosts_stats(&tv);
      }

      if(aggregationInfo.name) {
	free(aggregationInfo.name);
	aggregationInfo.name = NULL;
      }

      aggregationInfo.name = strdup(name);

      host->inc_num_queries_rcvd();
      host->set_tracked_host(aggregation_to_track);
      host->updateSeen();
      host->updateActivities();
      if(cli_host) host->incrContact(iface, host->get_host_serial(), cli_host->get_ip(), true, true);
    }
  }
}

/* *************************************** */

void Flow::processDetectedProtocol() {
  if(protocol_processed || (ndpi_flow == NULL)) return;

  switch(ndpi_detected_protocol) {
  case NDPI_PROTOCOL_DNS:
    if(ntop->getPrefs()->decode_dns_responses()) {
      if(ndpi_flow->host_server_name[0] != '\0') {
	char delimiter = '@', *name = NULL;
	char *at = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter);
	bool to_track = false;

	/* Consider only positive DNS replies */
	if(at != NULL)
	  name = &at[1], at[0] = '\0', to_track = true;
	else if((!strstr((const char*)ndpi_flow->host_server_name, ".in-addr.arpa"))
		&& (!strstr((const char*)ndpi_flow->host_server_name, ".ip6.arpa")))
	  name = (char*)ndpi_flow->host_server_name;

	if(name) {
	  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[DNS] %s", (char*)ndpi_flow->host_server_name);

	  if(ndpi_flow->protos.dns.ret_code != 0)
	    to_track = false; /* Error response */
	  else {
	    if(ndpi_flow->protos.dns.num_answers > 0) {
	      to_track = true, protocol_processed = true;

	      if(at != NULL)
		ntop->getRedis()->setResolvedAddress(name, (char*)ndpi_flow->host_server_name);
	    }
	  }

	  aggregateInfo((char*)ndpi_flow->host_server_name,
			ndpi_detected_protocol, aggregation_domain_name, to_track);
	}
      }
    }
    break;

  case NDPI_PROTOCOL_NETBIOS:
    if(ndpi_flow->host_server_name[0] != '\0')
      get_cli_host()->set_alternate_name((char*)ndpi_flow->host_server_name);
    break;

  case NDPI_PROTOCOL_WHOIS_DAS:
    if(ndpi_flow->host_server_name[0] != '\0') {
      protocol_processed = true;
      aggregateInfo((char*)ndpi_flow->host_server_name, ndpi_detected_protocol, aggregation_domain_name, true);
    }
    break;

  case NDPI_PROTOCOL_SSL:
  case NDPI_PROTOCOL_HTTP:
  case NDPI_PROTOCOL_HTTP_PROXY:
  case NDPI_SERVICE_GOOGLE:
    if(ndpi_flow->nat_ip[0] != '\0') {
      // ntop->getTrace()->traceEvent(TRACE_NORMAL, "-> %s", (char*)ndpi_flow->nat_ip);

      aggregateInfo((char*)ndpi_flow->nat_ip, ndpi_detected_protocol, aggregation_client_name, true);
    }

    if(ndpi_flow->host_server_name[0] != '\0') {
      char buf[64], *doublecol, delimiter = ':';
      u_int16_t sport = htons(cli_port), dport = htons(srv_port);
      Host *svr = (sport < dport) ? cli_host : srv_host;

      protocol_processed = true;

      /* if <host>:<port> We need to remove ':' */
      if((doublecol = (char*)strchr((const char*)ndpi_flow->host_server_name, delimiter)) != NULL)
	doublecol[0] = '\0';

      if(svr) {
	aggregateInfo((char*)ndpi_flow->host_server_name, ndpi_detected_protocol, aggregation_domain_name, true);

	if(ntop->getRedis()->getFlowCategory((char*)ndpi_flow->host_server_name,
					     buf, sizeof(buf), true) != NULL) {
	  categorization.flow_categorized = true;
	  categorization.category = strdup(buf);
	}

	if(ndpi_detected_protocol != NDPI_PROTOCOL_HTTP_PROXY) {
	  svr->setName((char*)ndpi_flow->host_server_name, true);
	  ntop->getRedis()->setResolvedAddress(svr->get_ip()->print(buf, sizeof(buf)),
					       (char*)ndpi_flow->host_server_name);
	}

	if(ndpi_flow->detected_os[0] != '\0') {
	  aggregateInfo((char*)ndpi_flow->detected_os, NTOPNG_NDPI_OS_PROTO_ID, aggregation_os_name, true);

	  if(cli_host)
	    cli_host->setOS((char*)ndpi_flow->detected_os);
	}
      }
    }
    break;
  } /* switch */

  if(protocol_processed
     /* For DNS we delay the memory free so that we can let nDPI analyze all the packets of the flow */
     && (ndpi_detected_protocol != NDPI_PROTOCOL_DNS))
    deleteFlowMemory();
}

/* *************************************** */

void Flow::guessProtocol() {
  detection_completed = true; /* We give up */

  /* We can guess the protocol */
  ndpi_detected_protocol = ndpi_guess_undetected_protocol(iface->get_ndpi_struct(), protocol,
							  ntohl(cli_host->get_ip()->get_ipv4()), ntohs(cli_port),
							  ntohl(srv_host->get_ip()->get_ipv4()), ntohs(srv_port));
}

/* *************************************** */

void Flow::setDetectedProtocol(u_int16_t proto_id) {
  if((ndpi_flow != NULL) || (!iface->is_ndpi_enabled())) {
    if(proto_id != NDPI_PROTOCOL_UNKNOWN) {
      ndpi_detected_protocol = proto_id;
      processDetectedProtocol();
      detection_completed = true;
    } else if((((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
	       && (cli_host != NULL)
	       && (srv_host != NULL))
	      || (!iface->is_ndpi_enabled())) {
      guessProtocol();
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

u_int64_t Flow::get_current_packets_cli2srv() {
  if((cli2srv_last_packets == 0) && (prev_cli2srv_last_packets == 0))
    return(cli2srv_packets);
  else {
    int64_t diff = cli2srv_packets - cli2srv_last_packets;

    if(diff > 0)
      return(diff);

    /*
       We need to do this as due to concurrency issues,
       we might have a negative value
    */
    diff = cli2srv_packets - prev_cli2srv_last_packets;

    if(diff > 0)
      return(diff);
    else
      return(0);
  }
};

/* *************************************** */

u_int64_t Flow::get_current_packets_srv2cli() {
  if((srv2cli_last_packets == 0) && (prev_srv2cli_last_packets == 0))
    return(srv2cli_packets);
  else {
    int64_t diff = srv2cli_packets - srv2cli_last_packets;

    if(diff > 0)
      return(diff);

    /*
       We need to do this as due to concurrency issues,
       we might have a negative value
    */
    diff = srv2cli_packets - prev_srv2cli_last_packets;

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
  lua_push_int_table_entry(vm,  "client.vlan", get_cli_host()->get_vlan_id());
  lua_push_str_table_entry(vm,  "server", get_srv_host()->get_ip()->print(buf, sizeof(buf)));
  lua_push_int_table_entry(vm,  "server.vlan", get_srv_host()->get_vlan_id());
  lua_push_int_table_entry(vm,  "sent", cli2srv_bytes);
  lua_push_int_table_entry(vm,  "rcvd", srv2cli_bytes);
  lua_push_int_table_entry(vm,  "sent.last", get_current_bytes_cli2srv());
  lua_push_int_table_entry(vm,  "rcvd.last", get_current_bytes_srv2cli());
  lua_push_int_table_entry(vm,  "duration", get_duration());

  lua_push_float_table_entry(vm, "client.latitude", get_cli_host()->get_latitude());
  lua_push_float_table_entry(vm, "client.longitude", get_cli_host()->get_longitude());
  lua_push_float_table_entry(vm, "server.latitude", get_srv_host()->get_latitude());
  lua_push_float_table_entry(vm, "server.longitude", get_srv_host()->get_longitude());

  if(verbose) {
    lua_push_bool_table_entry(vm, "client.private", get_cli_host()->get_ip()->isPrivateAddress());
    lua_push_str_table_entry(vm,  "client.country", get_cli_host()->get_country() ? get_cli_host()->get_country() : (char*)"");
    lua_push_bool_table_entry(vm, "server.private", get_srv_host()->get_ip()->isPrivateAddress());
    lua_push_str_table_entry(vm,  "server.country", get_srv_host()->get_country() ? get_srv_host()->get_country() : (char*)"");
    lua_push_str_table_entry(vm, "client.city", get_cli_host()->get_city() ? get_cli_host()->get_city() : (char*)"");
    lua_push_str_table_entry(vm, "server.city", get_srv_host()->get_city() ? get_srv_host()->get_city() : (char*)"");

    if(verbose) {
      if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
	 || (ndpi_detected_protocol != NDPI_PROTOCOL_UNKNOWN)
	 || iface->is_ndpi_enabled()
	 || iface->is_sprobe_interface())
	lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
      else
	lua_push_str_table_entry(vm, "proto.ndpi", (char*)CONST_TOO_EARLY);
    }
  }

  // Key
  /* Too slow */
#if 0
  snprintf(buf, sizeof(buf), "%s %s",
	   src->Host::get_name(buf1, sizeof(buf1), false),
	   dst->Host::get_name(buf2, sizeof(buf2), false));
#else
  /*Use the ip@vlan_id as a key only in case of multi vlan_id, otherwise use only the ip as a key*/
  if ((get_cli_host()->get_vlan_id() == 0) && (get_srv_host()->get_vlan_id() == 0)){
    snprintf(buf, sizeof(buf), "%s %s",
           intoaV4(ntohl(get_cli_ipv4()), buf1, sizeof(buf1)),
           intoaV4(ntohl(get_srv_ipv4()), buf2, sizeof(buf2)));
  } else {
    snprintf(buf, sizeof(buf), "%s@%d %s@%d",
           intoaV4(ntohl(get_cli_ipv4()), buf1, sizeof(buf1)),
           get_cli_host()->get_vlan_id(),
           intoaV4(ntohl(get_srv_ipv4()), buf2, sizeof(buf2)),
           get_srv_host()->get_vlan_id());
  }
#endif

  lua_pushstring(vm, buf);
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

char* Flow::print(char *buf, u_int buf_len) {
  char buf1[32], buf2[32];

  buf[0] = '\0';

  if((cli_host == NULL) || (srv_host == NULL)) return(buf);

  snprintf(buf, buf_len,
	   "%s %s:%u > %s:%u [proto: %u/%s][%u/%u pkts][%llu/%llu bytes]\n",
	   get_protocol_name(),
	   cli_host->get_ip()->print(buf1, sizeof(buf1)), ntohs(cli_port),
	   srv_host->get_ip()->print(buf2, sizeof(buf2)), ntohs(srv_port),
	   ndpi_detected_protocol,
	   ndpi_get_proto_name(iface->get_ndpi_struct(), ndpi_detected_protocol),
	   cli2srv_packets, srv2cli_packets,
	   (long long unsigned) cli2srv_bytes, (long long unsigned) srv2cli_bytes);

  return(buf);
}

/* *************************************** */

void Flow::update_hosts_stats(struct timeval *tv) {
  u_int64_t sent_packets, sent_bytes, rcvd_packets, rcvd_bytes;
  u_int64_t diff_sent_packets, diff_sent_bytes, diff_rcvd_packets, diff_rcvd_bytes;
  bool updated = false;

  sent_packets = cli2srv_packets, sent_bytes = cli2srv_bytes;
  diff_sent_packets = sent_packets - cli2srv_last_packets, diff_sent_bytes = sent_bytes - cli2srv_last_bytes;
  prev_cli2srv_last_bytes = cli2srv_last_bytes, prev_cli2srv_last_packets = cli2srv_last_packets;
  cli2srv_last_packets = sent_packets, cli2srv_last_bytes = sent_bytes;

  rcvd_packets = srv2cli_packets, rcvd_bytes = srv2cli_bytes;
  diff_rcvd_packets = rcvd_packets - srv2cli_last_packets, diff_rcvd_bytes = rcvd_bytes - srv2cli_last_bytes;
  prev_srv2cli_last_bytes = srv2cli_last_bytes, prev_srv2cli_last_packets = srv2cli_last_packets;
  srv2cli_last_packets = rcvd_packets, srv2cli_last_bytes = rcvd_bytes;

  if(cli_host)
    cli_host->incStats(protocol, ndpi_detected_protocol, diff_sent_packets, diff_sent_bytes,
		       diff_rcvd_packets, diff_rcvd_bytes);
  if(srv_host)
    srv_host->incStats(protocol, ndpi_detected_protocol, diff_rcvd_packets, diff_rcvd_bytes,
		       diff_sent_packets, diff_sent_bytes);

  if(aggregationInfo.name) {
    StringHost *host = iface->findHostByString(aggregationInfo.name, ndpi_detected_protocol, true);

    if(host)
      host->incStats(protocol, ndpi_detected_protocol, diff_rcvd_packets, diff_rcvd_bytes,
		     diff_sent_packets, diff_sent_bytes);
  }

  if(last_update_time.tv_sec > 0) {
    float tdiff_msec = ((float)(tv->tv_sec-last_update_time.tv_sec)*1000)+((tv->tv_usec-last_update_time.tv_usec)/(float)1000);

    if(tdiff_msec >= 1000 /* Do not updated when less than 1 second (1000 msec) */) {
      // bps
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

      // pps

      float pkts_msec = ((float)((cli2srv_last_packets-prev_cli2srv_last_packets)*1000))/tdiff_msec;

      if(pkts_msec < 0) pkts_msec = 0; /* Just to be safe */

      if(pkts_thpt < pkts_msec)      pkts_thpt_trend = trend_up;
      else if(pkts_thpt > pkts_msec) pkts_thpt_trend = trend_down;
      else                             pkts_thpt_trend = trend_stable;

      pkts_thpt = pkts_msec;

  /* ntop->getTrace()->traceEvent(TRACE_NORMAL, "[msec: %.1f][tdiff: %f][pkts: %u][pkts_thpt: %.2f pps]",
  pkts_msec,tdiff_msec, (cli2srv_last_packets-prev_cli2srv_last_packets),
  (pkts_thpt)); */

      updated = true;
    }
  } else
    updated = true;

  if(updated)
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

json_object* Flow::processJson(ProcessInfo *proc) {
  json_object *inner;

  inner = json_object_new_object();
  json_object_object_add(inner, "pid", json_object_new_int64(proc->pid));
  json_object_object_add(inner, "father_pid", json_object_new_int64(proc->father_pid));
  json_object_object_add(inner, "name", json_object_new_string(proc->name));
  json_object_object_add(inner, "father_name", json_object_new_string(proc->father_name));
  json_object_object_add(inner, "user_name", json_object_new_string(proc->user_name));
  json_object_object_add(inner, "actual_memory", json_object_new_int(proc->actual_memory));
  json_object_object_add(inner, "peak_memory", json_object_new_int(proc->peak_memory));
  json_object_object_add(inner, "average_cpu_load", json_object_new_double(proc->average_cpu_load));
  json_object_object_add(inner, "num_vm_page_faults", json_object_new_int(proc->num_vm_page_faults));

  return(inner);
}

/* *************************************** */

void Flow::processLua(lua_State* vm, ProcessInfo *proc, bool client) {
  lua_newtable(vm);

  lua_push_int_table_entry(vm, "pid", proc->pid);
  lua_push_int_table_entry(vm, "father_pid", proc->father_pid);
  lua_push_str_table_entry(vm, "name", proc->name);
  lua_push_str_table_entry(vm, "father_name", proc->father_name);
  lua_push_str_table_entry(vm, "user_name", proc->user_name);
  lua_push_int_table_entry(vm, "actual_memory", proc->actual_memory);
  lua_push_int_table_entry(vm, "peak_memory", proc->peak_memory);
  lua_push_float_table_entry(vm, "average_cpu_load", proc->average_cpu_load);
  lua_push_int_table_entry(vm, "num_vm_page_faults", proc->num_vm_page_faults);

  lua_pushstring(vm, client ? "client_process" : "server_process");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

void Flow::lua(lua_State* vm, bool detailed_dump) {
  char buf[64];

  lua_newtable(vm);

  if(get_cli_host()) {
    if(detailed_dump) lua_push_str_table_entry(vm, "cli.host", get_cli_host()->get_name(buf, sizeof(buf), false));
    lua_push_int_table_entry(vm, "cli.source_id", get_cli_host()->getSourceId());
    lua_push_str_table_entry(vm, "cli.ip", get_cli_host()->get_ip()->print(buf, sizeof(buf)));
    lua_push_bool_table_entry(vm, "cli.systemhost", get_cli_host()->isSystemHost());
    lua_push_int32_table_entry(vm, "cli.network_id", get_cli_host()->get_local_network_id());
  } else {
    lua_push_nil_table_entry(vm, "cli.host");
    lua_push_nil_table_entry(vm, "cli.ip");
  }

  lua_push_int_table_entry(vm, "cli.port", get_cli_port());

  if(get_srv_host()) {
    if(detailed_dump) lua_push_str_table_entry(vm, "srv.host", get_srv_host()->get_name(buf, sizeof(buf), false));
    lua_push_int_table_entry(vm, "srv.source_id", get_cli_host()->getSourceId());
    lua_push_str_table_entry(vm, "srv.ip", get_srv_host()->get_ip()->print(buf, sizeof(buf)));
    lua_push_bool_table_entry(vm, "srv.systemhost", get_srv_host()->isSystemHost());
    lua_push_int32_table_entry(vm, "srv.network_id", get_srv_host()->get_local_network_id());
  } else {
    lua_push_nil_table_entry(vm, "srv.host");
    lua_push_nil_table_entry(vm, "srv.ip");
  }

  lua_push_int_table_entry(vm, "srv.port", get_srv_port());
  lua_push_int_table_entry(vm, "vlan", get_vlan_id());
  lua_push_str_table_entry(vm, "proto.l4", get_protocol_name());

  if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
     || (ndpi_detected_protocol != NDPI_PROTOCOL_UNKNOWN)
     || iface->is_ndpi_enabled()
     || iface->is_sprobe_interface()) {
    lua_push_str_table_entry(vm, "proto.ndpi", get_detected_protocol_name());
  } else
    lua_push_str_table_entry(vm, "proto.ndpi", (char*)CONST_TOO_EARLY);

  lua_push_int_table_entry(vm, "bytes", cli2srv_bytes+srv2cli_bytes);
  lua_push_int_table_entry(vm, "bytes.last", get_current_bytes_cli2srv() + get_current_bytes_srv2cli());
  lua_push_int_table_entry(vm, "packets", cli2srv_packets+srv2cli_packets);
  lua_push_int_table_entry(vm, "packets.last", get_current_packets_cli2srv() + get_current_packets_srv2cli());
  lua_push_int_table_entry(vm, "seen.first", get_first_seen());
  lua_push_int_table_entry(vm, "seen.last", get_last_seen());
  lua_push_int_table_entry(vm, "duration", get_duration());

  lua_push_int_table_entry(vm, "cli2srv.bytes", cli2srv_bytes);
  lua_push_int_table_entry(vm, "srv2cli.bytes", srv2cli_bytes);

  lua_push_int_table_entry(vm, "cli2srv.packets", cli2srv_packets);
  lua_push_int_table_entry(vm, "srv2cli.packets", srv2cli_packets);

  if(detailed_dump) {
    lua_push_int_table_entry(vm, "tcp_flags", getTcpFlags());
    lua_push_str_table_entry(vm, "category", categorization.category ? categorization.category : (char*)"");
  }

  lua_push_str_table_entry(vm, "moreinfo.json", get_json_info());

  if(client_proc) processLua(vm, client_proc, true);
  if(server_proc) processLua(vm, server_proc, false);

  lua_push_float_table_entry(vm, "throughput_bps", bytes_thpt);
  lua_push_int_table_entry(vm, "throughput_trend_bps", bytes_thpt_trend);
  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[bytes_thpt: %.2f] [bytes_thpt_trend: %d]", bytes_thpt,bytes_thpt_trend);

  lua_push_float_table_entry(vm, "throughput_pps", pkts_thpt);
  lua_push_int_table_entry(vm, "throughput_trend_pps", pkts_thpt_trend);
  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[pkts_thpt: %.2f] [pkts_thpt_trend: %d]", pkts_thpt,pkts_thpt_trend);

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

  return(isIdle(ntop->getPrefs()->get_flow_max_idle()));
};

/* *************************************** */

bool Flow::isFlowPeer(char *numIP, u_int16_t vlanId) {
  char s_buf[32], *ret;

  if((!cli_host) || (!srv_host)) return(false);

  ret = cli_host->get_ip()->print(s_buf, sizeof(s_buf));
  if ((strcmp(ret, numIP) == 0) &&
     (cli_host->get_vlan_id() == vlanId))return(true);

  ret = srv_host->get_ip()->print(s_buf, sizeof(s_buf));
  if ((strcmp(ret, numIP) == 0) &&
     (cli_host->get_vlan_id() == vlanId))return(true);

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
  stats-> incStats(ndpi_detected_protocol,
		   cli2srv_packets, cli2srv_bytes,
		   srv2cli_packets, srv2cli_bytes);
}

/* *************************************** */

char* Flow::serialize() {
  json_object *my_object, *inner;
  char *rsp, buf[64], jsonbuf[64];

  my_object = json_object_new_object();
  json_object_object_add(my_object, Utils::jsonLabel(IPV4_SRC_ADDR,"IPV4_SRC_ADDR", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_string(cli_host->get_string_key(buf, sizeof(buf))));
  json_object_object_add(my_object, Utils::jsonLabel(L4_SRC_PORT,"L4_SRC_PORT", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int(get_cli_port()));

  json_object_object_add(my_object, Utils::jsonLabel(IPV4_DST_ADDR,"IPV4_DST_ADDR", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_string(srv_host->get_string_key(buf, sizeof(buf))));
  json_object_object_add(my_object, Utils::jsonLabel(L4_DST_PORT,"L4_DST_PORT", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int(get_srv_port()));

  json_object_object_add(my_object, Utils::jsonLabel(PROTOCOL,"PROTOCOL", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int(protocol));

  json_object_object_add(my_object, Utils::jsonLabel(SRC_VLAN,"SRC_VLAN", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int(cli_host->get_vlan_id()));
  json_object_object_add(my_object, Utils::jsonLabel(DST_VLAN,"DST_VLAN", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int(srv_host->get_vlan_id()));

  if(((cli2srv_packets+srv2cli_packets) > NDPI_MIN_NUM_PACKETS)
     || (ndpi_detected_protocol != NDPI_PROTOCOL_UNKNOWN))
    json_object_object_add(my_object, Utils::jsonLabel(L7_PROTO_NAME,"L7_PROTO_NAME", jsonbuf, sizeof(jsonbuf)),
			   json_object_new_string(get_detected_protocol_name()));

  if(protocol == IPPROTO_TCP)
    json_object_object_add(my_object, Utils::jsonLabel(TCP_FLAGS,"TCP_FLAGS", jsonbuf, sizeof(jsonbuf)),
			   json_object_new_int(tcp_flags));

  json_object_object_add(my_object, Utils::jsonLabel(OUT_PKTS,"OUT_PKTS", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int64(cli2srv_packets));
  json_object_object_add(my_object, Utils::jsonLabel(OUT_BYTES,"OUT_BYTES", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int64(cli2srv_bytes));

  json_object_object_add(my_object, Utils::jsonLabel(IN_PKTS,"IN_PKTS", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int64(srv2cli_packets));
  json_object_object_add(my_object, Utils::jsonLabel(IN_BYTES,"IN_BYTES", jsonbuf, sizeof(jsonbuf)),
			 json_object_new_int64(srv2cli_bytes));

  if(json_info && strcmp(json_info, "{}")) json_object_object_add(my_object, "json", json_object_new_string(json_info));


  if (0) {
    inner = json_object_new_object();
    json_object_object_add(inner, "first", json_object_new_int((u_int32_t)first_seen));
    json_object_object_add(inner, "last", json_object_new_int((u_int32_t)last_seen));
    json_object_object_add(my_object, "seen", inner);

    if(vlanId > 0) json_object_object_add(my_object, "vlanId", json_object_new_int(vlanId));

    json_object_object_add(my_object, "throughput_bps", json_object_new_double(bytes_thpt));
    json_object_object_add(my_object, "throughput_trend_bps", json_object_new_string(Utils::trend2str(bytes_thpt_trend)));

    json_object_object_add(my_object, "throughput_pps", json_object_new_double(pkts_thpt));
    json_object_object_add(my_object, "throughput_trend_pps", json_object_new_string(Utils::trend2str(pkts_thpt_trend)));

    if(categorization.flow_categorized) json_object_object_add(my_object, "category", json_object_new_string(categorization.category));

    if(client_proc != NULL)
      json_object_object_add(my_object, "process_client", processJson(client_proc));

    if(server_proc != NULL)
      json_object_object_add(my_object, "process_server", processJson(server_proc));
  }

  /* JSON string */
  rsp = strdup(json_object_to_json_string(my_object));
  ntop->getTrace()->traceEvent(TRACE_DEBUG, "Emitting Flow: %s", rsp);

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

/* *************************************** */

void Flow::updateTcpFlags(time_t when, u_int8_t flags) {
  if((flags == TH_SYN)
	  && (tcp_flags == TH_SYN) /* SYN was already received */
	  && (cli2srv_packets > 2 /* We tolerate two SYN at the beginning of the connection */)
	  && ((last_seen-first_seen) < 2 /* (sec) SYN flood must be quick */)
	  && cli_host)
    cli_host->updateSynFlags(when, flags, this);

  /* The update below must be after the above check */
  tcp_flags |= flags;
}

/* *************************************** */

void Flow::handle_process(ProcessInfo *pinfo, bool client_process) {
  ProcessInfo *proc;

  if(client_process) {
    if(client_proc)
      memcpy(client_proc, pinfo, sizeof(ProcessInfo));
    else {
      if((proc = new ProcessInfo) == NULL) return;
      memcpy(proc, pinfo, sizeof(ProcessInfo));
      client_proc = proc, cli_host->setSystemHost(); /* Outgoing */
    }
  } else {
    if(server_proc)
      memcpy(server_proc, pinfo, sizeof(ProcessInfo));
    else {
      if((proc = new ProcessInfo) == NULL) return;
      memcpy(proc, pinfo, sizeof(ProcessInfo));      
      server_proc = proc, srv_host->setSystemHost();  /* Incoming */
    }
  }
}

/* *************************************** */

void Flow::refresh_process_peer(Host *host, u_int16_t port, bool as_client) {
  ProcessInfo p;
  char _port[32], rsp[128], *pid, *process_name, *w, path[MAX_PATH];
  FILE *f;

  snprintf(_port, sizeof(_port), "%s.%u", SPROBE_HASH_NAME, port);
  if(ntop->getRedis()->get(_port, rsp, sizeof(rsp)) == -1)
    return;

  /* <PID>,<process name> */
  if((pid = strtok_r(rsp, ",", &w)) == NULL) return;
  if((process_name = strtok_r(NULL, ",", &w)) == NULL) return;

  snprintf(path, sizeof(path), "/proc/%s/status", pid);

  memset(&p, 0, sizeof(p));
  snprintf(p.name, sizeof(p.name), "%s", process_name);
  p.pid = atol(pid);

  if((f = fopen(path, "r")) != NULL) {
    char *line, buf[128];

    while((line = fgets(buf, sizeof(buf), f)) != NULL) {
      buf[strlen(buf)-1] = '\0';

      if(strncmp(line, "Name:", 5) == 0) {
	snprintf(p.name, sizeof(p.name), "%s", &buf[6]);
      } else if(strncmp(line, "PPid:", 5) == 0) {
        p.father_pid = atol(&line[6]);
      } else if(strncmp(line, "Uid:", 4) == 0) {
	struct passwd *pwd;
        int uid = atoi(&buf[5]);

	if((uid >= 0) && ((pwd = getpwuid(uid)) != NULL))
	  snprintf(p.user_name, sizeof(p.user_name), "%s", pwd->pw_name);
      }
    }

    fclose(f);

    if(p.father_pid > 0) {
      snprintf(path, sizeof(path), "/proc/%u/status", p.father_pid);

      if((f = fopen(path, "r")) != NULL) {
	char *line, buf[128];

	while((line = fgets(buf, sizeof(buf), f)) != NULL) {
	  buf[strlen(buf)-1] = '\0';

	  if(strncmp(line, "Name:", 5) == 0) {
	    snprintf(p.father_name, sizeof(p.father_name), "%s", &buf[6]);
	    break;
	  }
	}

	fclose(f);
      }
    }

    if(0) {
      char buf1[32], buf2[32];

      ntop->getTrace()->traceEvent(TRACE_NORMAL, "[port: %u][%s][%s:%u <-> %s:%u][%s]",
				   port, as_client ? "CLIENT" : "SERVER",
				   cli_host->get_ip()->print(buf1, sizeof(buf1)), ntohs(cli_port),
				   srv_host->get_ip()->print(buf2, sizeof(buf2)), ntohs(srv_port),
				   p.name);
    }
  }

  // !handle_process(&p, as_client);
}

/* *************************************** */

void Flow::refresh_process() {
  if(!iface->is_sprobe_interface()) return;

  if(srv_port && cli_port) {
    if(cli_host && cli_host->isSystemHost())
      refresh_process_peer(cli_host, ntohs(cli_port), true);

    if(srv_host && srv_host->isSystemHost())
      refresh_process_peer(srv_host, ntohs(srv_port), false);
  }
}

/* *************************************** */

u_int32_t Flow::getPid(bool client) {
  ProcessInfo *proc = client ? client_proc : server_proc;

  return((proc == NULL) ? 0 : proc->pid);
};

/* *************************************** */

u_int32_t Flow::getFatherPid(bool client) {
  ProcessInfo *proc = client ? client_proc : server_proc;

  return((proc == NULL) ? 0 : proc->father_pid);
};

/* *************************************** */

char* Flow::get_username(bool client) {
  ProcessInfo *proc = client ? client_proc : server_proc;

  return((proc == NULL) ? NULL : proc->user_name);
};

/* *************************************** */

char* Flow::get_proc_name(bool client) {
  ProcessInfo *proc = client ? client_proc : server_proc;

  return((proc == NULL) ? NULL : proc->name);
};
