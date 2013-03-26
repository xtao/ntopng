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

Flow::Flow(NtopGlobals *globals,
	   u_int16_t _vlanId, u_int8_t _protocol, 
	   u_int32_t _lower_ip, u_int16_t _lower_port,
	   u_int32_t _upper_ip, u_int16_t _upper_port) {
  Flow(_vlanId, _protocol, _lower_ip, _lower_port, _upper_ip, _upper_port); 
  allocFlowMemory(globals);
}

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

void Flow::allocFlowMemory(NtopGlobals *globals) {
  if((ndpi_flow = (ndpi_flow_struct*)calloc(1, globals->get_flow_size())) == NULL)
    throw "Not enough memory";  
  
  if((src_id = calloc(1, globals->get_size_id())) == NULL)
    throw "Not enough memory";  
  
  if((dst_id = calloc(1, globals->get_size_id())) == NULL)
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
