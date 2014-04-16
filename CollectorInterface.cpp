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

#ifdef __APPLE__
#include <uuid/uuid.h>
#endif

#define MSG_VERSION 0

struct zmq_msg_hdr {
  char url[32];
  u_int32_t version;
  u_int32_t size;
};

/* **************************************************** */

CollectorInterface::CollectorInterface(const char *_endpoint, const char *_topic)
  : NetworkInterface(_endpoint) {
  char *slash, *tmp, *e;

  num_drops = 0, num_subscribers = 0;
  topic = strdup(_topic);

  /*
    We need to cleanup the interface name
    Format <tcp|udp>://<host>:<port>
  */
  if((slash = strchr(ifname, '/')) != NULL) {
    char buf[64], *comma;
    int i = 1, len;

    while(slash[i] == '/') i++;

    snprintf(buf, sizeof(buf), "zmq@%s", &slash[i]);
    free(ifname);

    comma = strchr(buf, ',');

    if(comma != NULL) {
      len = strlen(buf);
      if(len > 16) buf[16] = '.', buf[17] = '.', buf[18] = '.', buf[19] = '\0';
    }

    ifname = strdup(buf);
  }
  
  context = zmq_ctx_new();

  if((tmp = strdup(_endpoint)) == NULL) throw("Out of memory");

  e = strtok(tmp, ",");
  while(e != NULL) {
    if(num_subscribers == CONST_MAX_NUM_ZMQ_SUBSCRIBERS) {
      ntop->getTrace()->traceEvent(TRACE_ERROR, 
				   "Too many endpoints defined %u: skipping those in excess", 
				   num_subscribers);
      break;
    }

    subscriber[num_subscribers].socket = zmq_socket(context, ZMQ_SUB);

    if(zmq_connect(subscriber[num_subscribers].socket, e) != 0) {
      zmq_close(subscriber[num_subscribers].socket);
      zmq_ctx_destroy(context);
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to connect to ZMQ endpoint %s", e);
      throw("Unable to connect to the specified ZMQ endpoint");
    }

    if(zmq_setsockopt(subscriber[num_subscribers].socket, ZMQ_SUBSCRIBE, topic, strlen(topic)) != 0) {
      zmq_close(subscriber[num_subscribers].socket);
      zmq_ctx_destroy(context);
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to connect to the specified ZMQ endpoint");
      throw("Unable to subscribe to the specified ZMQ endpoint");
    }

    subscriber[num_subscribers].endpoint = strdup(e);

    num_subscribers++;
    e = strtok(NULL, ",");
  }

  free(tmp);
}

/* **************************************************** */

CollectorInterface::~CollectorInterface() {  
  for(int i=0; i<num_subscribers; i++) {
    if(subscriber[i].endpoint) free(subscriber[i].endpoint);
    zmq_close(subscriber[i].socket);
  }

  if(topic) free(topic);
  zmq_ctx_destroy(context);
}

/* **************************************************** */

void CollectorInterface::collect_flows() {
  struct zmq_msg_hdr h;
  char payload[8192];
  u_int payload_len = sizeof(payload)-1;
  zmq_pollitem_t items[CONST_MAX_NUM_ZMQ_SUBSCRIBERS];
  int rc, size;

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Collecting flows...");

  while(isRunning()) {
    for(int i=0; i<num_subscribers; i++)
      items[i].socket = subscriber[i].socket, items[i].fd = 0, items[i].events = ZMQ_POLLIN, items[i].revents = 0;

    do {
      rc = zmq_poll(items, num_subscribers, 1000);
      if((rc < 0) || (!isRunning())) return;
    } while(rc == 0);

    for(int source_id=0; source_id<num_subscribers; source_id++) {
      if(items[source_id].revents & ZMQ_POLLIN) {
	size = zmq_recv(items[source_id].socket, &h, sizeof(h), 0);

	if((size != sizeof(h)) || (h.version != MSG_VERSION)) {
	  ntop->getTrace()->traceEvent(TRACE_WARNING,
				       "Unsupported publisher version [%d]: your nProbe sender is outdated?",
				       h.version);
	  continue;
	}

	size = zmq_recv(items[source_id].socket, payload, payload_len, 0);

	if(size > 0) {
	  json_object *o;
	  ZMQ_Flow flow;

	  payload[size] = '\0';
	  o = json_tokener_parse(payload);

	  if(o != NULL) {
	    struct json_object_iterator it = json_object_iter_begin(o);
	    struct json_object_iterator itEnd = json_object_iter_end(o);

	    /* Reset data */
	    memset(&flow, 0, sizeof(flow));
	    flow.additional_fields = json_object_new_object();
	    flow.pkt_sampling_rate = 1; /* 1:1 (no sampling) */
	    flow.source_id = source_id;

	    // flow.vlan_id = source_id;

	    while(!json_object_iter_equal(&it, &itEnd)) {
	      const char *key   = json_object_iter_peek_name(&it);
	      json_object *v    = json_object_iter_peek_value(&it);
	      const char *value = json_object_get_string(v);

	      if((key != NULL) && (value != NULL)) {
		u_int key_id = atoi(key);

		ntop->getTrace()->traceEvent(TRACE_INFO, "[%s]=[%s]", key, value);

		switch(key_id) {
		case IN_SRC_MAC:
		  /* Format 00:00:00:00:00:00 */
		  sscanf(value, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			 &flow.src_mac[0], &flow.src_mac[1], &flow.src_mac[2],
			 &flow.src_mac[3], &flow.src_mac[4], &flow.src_mac[5]);
		  break;
		case OUT_DST_MAC:
		  sscanf(value, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			 &flow.dst_mac[0], &flow.dst_mac[1], &flow.dst_mac[2],
			 &flow.dst_mac[3], &flow.dst_mac[4], &flow.dst_mac[5]);
		  break;
		case IPV4_SRC_ADDR:
		case IPV6_SRC_ADDR:
		  flow.src_ip.set_from_string((char*)value);
		  break;
		case IPV4_DST_ADDR:
		case IPV6_DST_ADDR:
		  flow.dst_ip.set_from_string((char*)value);
		  break;
		case L4_SRC_PORT:
		  flow.src_port = htons(atoi(value));
		  break;
		case L4_DST_PORT:
		  flow.dst_port = htons(atoi(value));
		  break;
		case SRC_VLAN:
		case DST_VLAN:
		  flow.vlan_id = atoi(value);
		  break;
		case L7_PROTO:
		  flow.l7_proto = atoi(value);
		  break;
		case PROTOCOL:
		  flow.l4_proto = atoi(value);
		  break;
		case TCP_FLAGS:
		  flow.tcp_flags = atoi(value);
		  break;
		case IN_PKTS:
		  flow.in_pkts = atol(value);
		  break;
		case IN_BYTES:
		  flow.in_bytes = atol(value);
		  break;
		case OUT_PKTS:
		  flow.out_pkts = atol(value);
		  break;
		case OUT_BYTES:
		  flow.out_bytes = atol(value);
		  break;
		case FIRST_SWITCHED:
		  flow.first_switched = atol(value);
		  break;
		case LAST_SWITCHED:
		  flow.last_switched = atol(value);
		  break;
		case SAMPLING_INTERVAL:
		  flow.pkt_sampling_rate = atoi(value);
		  break;
		case DIRECTION:
		  flow.direction = atoi(value);
		  break;
		case EPP_REGISTRAR_NAME:
		  snprintf(flow.epp_registrar_name, sizeof(flow.epp_registrar_name), "%s", value);
		  break;
		case EPP_CMD:
		  flow.epp_cmd = atoi(value);
		  break;
		case EPP_CMD_ARGS:
		  snprintf(flow.epp_cmd_args, sizeof(flow.epp_cmd_args), "%s", value);
		  break;
		case EPP_RSP_CODE:
		  flow.epp_rsp_code = atoi(value);
		  break;
		case EPP_REASON_STR:
		  snprintf(flow.epp_reason_str, sizeof(flow.epp_reason_str), "%s", value);
		  break;
		case EPP_SERVER_NAME:
		  snprintf(flow.epp_server_name, sizeof(flow.epp_server_name), "%s", value);
		  break;
		case PROC_CPU_ID:
		  flow.process.cpu_id = atoi(value);
		  break;
		case PROC_ID:
		  sprobe_interface = true; /* We're collecting system flows */
		  flow.process.pid = atoi(value);
		  break;
		case PROC_NAME:
		  snprintf(flow.process.name, sizeof(flow.process.name), "%s", value);
		  break;
		case PROC_FATHER_ID:
		  flow.process.father_pid = atoi(value);
		  break;
		case PROC_FATHER_NAME:
		  snprintf(flow.process.father_name, sizeof(flow.process.father_name), "%s", value);
		  break;
		case PROC_USER_NAME:
		  snprintf(flow.process.user_name, sizeof(flow.process.user_name), "%s", value);
		  break;
		case PROC_ACTUAL_MEMORY:
		  flow.process.actual_memory = atoi(value);
		  break;
		case PROC_PEAK_MEMORY:
		  flow.process.peak_memory = atoi(value);
		  break;
		case PROC_AVERAGE_CPU_LOAD:
		  flow.process.average_cpu_load = (u_int8_t)atoi(value);
		  break;
		case PROC_NUM_PAGE_FAULTS:
		  flow.process.num_vm_page_faults = atoi(value);
		  break;
		default:
		  ntop->getTrace()->traceEvent(TRACE_INFO, "Not handled ZMQ field %u", key_id);
		  json_object_object_add(flow.additional_fields, key, json_object_new_string(value));
		  break;
		}
	      }

	      /* Move to the next element */
	      json_object_iter_next(&it);
	    }

	    /* Set default fields for EPP */
	    if(flow.epp_cmd > 0) {
	      if(flow.dst_port == 0) flow.dst_port = 443;
	      if(flow.src_port == 0) flow.dst_port = 1234;
	      flow.l4_proto = IPPROTO_TCP;
	      flow.in_pkts = flow.out_pkts = 1; /* Dummy */
	      flow.l7_proto = NDPI_PROTOCOL_EPP;
	    }

	    /* Process Flow */
	    flow_processing(&flow);

	    /* Dispose memory */
	    json_object_put(o);
	    json_object_put(flow.additional_fields);
	  } else {
	    ntop->getTrace()->traceEvent(TRACE_WARNING,
					 "Invalid message received: your nProbe sender is outdated?");
	    ntop->getTrace()->traceEvent(TRACE_WARNING, "[%u] %s", h.size, payload);
	  }

	  ntop->getTrace()->traceEvent(TRACE_INFO, "[%u] %s", h.size, payload);
	}
      }
    } /* for */
  }

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Flow collection is over.");
}

/* **************************************************** */

static void* packetPollLoop(void* ptr) {
  CollectorInterface *iface = (CollectorInterface*)ptr;

  /* Wait until the initialization completes */
  while(!iface->isRunning()) sleep(1);

  iface->collect_flows();
  return(NULL);
}

/* **************************************************** */

void CollectorInterface::startPacketPolling() {
  pthread_create(&pollLoop, NULL, packetPollLoop, (void*)this);
  NetworkInterface::startPacketPolling();
}

/* **************************************************** */

void CollectorInterface::shutdown() {
  void *res;

  if(running) {
    NetworkInterface::shutdown();
    pthread_join(pollLoop, &res);
  }
}

/* **************************************************** */

bool CollectorInterface::set_packet_filter(char *filter) {
  ntop->getTrace()->traceEvent(TRACE_ERROR,
			       "No filter can be set on a collector interface. Ignored %s", filter);
  return(false);
}

/* **************************************************** */
