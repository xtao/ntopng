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

/* *************************************** */

Flow::Flow(u_int16_t _vlanId, u_int8_t _protocol, 
	   u_int32_t _lower_ip, u_int16_t _lower_port,
	   u_int32_t _upper_ip, u_int16_t _upper_port) {
  vlanId = _vlanId, protocol = _protocol,
    lower_ip = _lower_ip, lower_port = _lower_port,
    upper_ip = _upper_ip, upper_port = _upper_port;
  packets = bytes = 0, detection_completed = false, detected_protocol = NDPI_PROTOCOL_UNKNOWN;
  ndpi_flow = NULL, src_id = dst_id = NULL;
}

/* *************************************** */

void Flow::allocFlowMemory() {
  if((ndpi_flow = (ndpi_flow_struct*)calloc(1, ntopGlobals->get_flow_size())) == NULL)
    throw "Not enough memory";  
  
  if((src_id = calloc(1, ntopGlobals->get_size_id())) == NULL)
    throw "Not enough memory";  
  
  if((dst_id = calloc(1, ntopGlobals->get_size_id())) == NULL)
    throw "Not enough memory";  
}

/* *************************************** */

void Flow::deleteFlowMemory() {
  if(ndpi_flow) free(ndpi_flow);
  if(src_id)    free(src_id);
  if(dst_id)    free(dst_id);
}

/* *************************************** */

Flow::~Flow() {
  deleteFlowMemory();
}

/* *************************************** */

void Flow::setDetectedProtocol(u_int16_t proto_id, u_int8_t l4_proto) {
  if(proto_id != NDPI_PROTOCOL_UNKNOWN) {
    detected_protocol = proto_id;
    
    if((detected_protocol != NDPI_PROTOCOL_UNKNOWN)
       || (l4_proto == IPPROTO_UDP)
       || ((l4_proto == IPPROTO_TCP) && (packets > 10))) {
      detection_completed = true;
      deleteFlowMemory();
    }
  }
}

/* *************************************** */

int Flow::compare(Flow *fb) {
  if(vlanId < fb->vlanId) return(-1); else { if(vlanId > fb->vlanId) return(1); }
  if(lower_ip < fb->lower_ip) return(-1); else { if(lower_ip > fb->lower_ip) return(1); }
  if(lower_port < fb->lower_port) return(-1); else { if(lower_port > fb->lower_port) return(1); }
  if(upper_ip < fb->upper_ip) return(-1); else { if(upper_ip > fb->upper_ip) return(1); }
  if(upper_port < fb->upper_port) return(-1); else { if(upper_port > fb->upper_port) return(1); }
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

  /* Convert the string to lowercase */
  retStr = (char*)(cp+1);

  return(retStr);
}

/* *************************************** */

void Flow::print() {
  char buf1[32], buf2[32];

  printf("\t%s %s:%u > %s:%u [proto: %u/%s][%u pkts/%u bytes]\n",
	 ipProto2Name(protocol),
	 intoaV4(ntohl(lower_ip), buf1, sizeof(buf1)),
	 ntohs(lower_port),
	 intoaV4(ntohl(upper_ip), buf2, sizeof(buf2)),
	 ntohs(upper_port),
	 detected_protocol,
	 ndpi_get_proto_name(ntopGlobals->get_ndpi_struct(), detected_protocol),
	 packets, bytes);
}
