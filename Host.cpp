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

Host::Host(NetworkInterface *_iface) : GenericHost(_iface) {
  ip = new IpAddress();
  initialize(NULL, 0, false);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId, IpAddress *_ip) : GenericHost(_iface) {
  ip = new IpAddress(_ip);
  initialize(mac, _vlanId, true);
}

/* *************************************** */

Host::Host(NetworkInterface *_iface, u_int8_t mac[6], u_int16_t _vlanId) : GenericHost(_iface) {
  ip = NULL;
  initialize(mac, _vlanId, true);
}

/* *************************************** */

Host::~Host() {
#if 0
  if(ndpiStats) {
    ndpiStats->print(iface);
    //printf("%s\n", ndpiStats->serialize());
    ndpiStats->deserialize(ndpiStats->serialize());        
    //ndpiStats->serialize());
  }
#endif

  if(localHost && (ip != NULL)) {
    char key[128], buf[64], *k;

    k = ip->print(buf, sizeof(buf));

    snprintf(key, sizeof(key), "%s.client", k);
    ntop->getRedis()->del(key);

    snprintf(key, sizeof(key), "%s.server", k);
    ntop->getRedis()->del(key);
  }

  if(symbolic_name) free(symbolic_name);
  if(country)       free(country);
  if(city)          free(city);
  if(asname)        free(asname);
  delete ip;
  delete m;
}

/* *************************************** */

void Host::initialize(u_int8_t mac[6], u_int16_t _vlanId, bool init_all) {
  if(mac) memcpy(mac_address, mac, 6); else memset(mac_address, 0, 6);

  category[0] = '\0';
  num_uses = 0, symbolic_name = NULL, vlan_id = _vlanId;
  first_seen = last_seen = iface->getTimeLastPktRcvd();
  m = new Mutex();
  localHost = false, asn = 0, asname = NULL, country = NULL, city = NULL;

  if(init_all) {
    if(ip) {
      char buf[64], rsp[256], *host = ip->print(buf, sizeof(buf));
      
      updateLocal();

      if(localHost || ntop->getPrefs()->is_dns_resolution_enabled_for_all_hosts()) {
	if(ntop->getRedis()->getAddress(host, rsp, sizeof(rsp), true) == 0)
	  symbolic_name = strdup(rsp);
	else
	  ntop->getRedis()->queueHostToResolve(host, false, localHost);
      }

      ntop->getGeolocation()->getAS(ip, &asn, &asname);
      ntop->getGeolocation()->getInfo(ip, &country, &city, &latitude, &longitude);      
    } else {
      char buf[32];

      snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
	       mac_address[0], mac_address[1], mac_address[2], 
	       mac_address[3], mac_address[4], mac_address[5]);
      
      symbolic_name = strdup(buf);
      localHost = true;
    }
  }
}

/* *************************************** */

void Host::updateLocal() {
  localHost = ip->isLocalHost();
  
  if(0) {
    char buf[64];
    
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s is %s", 
				 ip->print(buf, sizeof(buf)), localHost ? "local" : "remote");
  }
}

/* *************************************** */

char* Host::get_mac(char *buf, u_int buf_len) {
  snprintf(buf, buf_len,
	   "%02X:%02X:%02X:%02X:%02X:%02X",
	   mac_address[0] & 0xFF, mac_address[1] & 0xFF,
	   mac_address[2] & 0xFF, mac_address[3] & 0xFF,
	   mac_address[4] & 0xFF, mac_address[5] & 0xFF);

  return(buf);
}

/* *************************************** */

void Host::getHostContacts(lua_State* vm) {

  if(get_ip() == NULL) return;

  lua_newtable(vm);

  /* client */
  ntop->getRedis()->getHostContacts(vm, this, true /* client */);
  lua_pushstring(vm, "client");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  /* server */
  ntop->getRedis()->getHostContacts(vm, this, false /* server */);
  lua_pushstring(vm, "server");
  lua_insert(vm, -2);
  lua_settable(vm, -3);

  lua_pushstring(vm, "contacts");
  lua_insert(vm, -2);
  lua_settable(vm, -3);
}

/* *************************************** */

void Host::lua(lua_State* vm, bool host_details, bool verbose, bool returnHost) {
  char buf[64];

  if(host_details) {
    lua_newtable(vm);

    lua_push_bool_table_entry(vm, "localhost", isLocalHost());

    if(ip)
      lua_push_str_table_entry(vm, "ip", ip->print(buf, sizeof(buf)));
    else
      lua_push_nil_table_entry(vm, "ip");

    lua_push_str_table_entry(vm, "mac", get_mac(buf, sizeof(buf)));
    lua_push_str_table_entry(vm, "name", get_name(buf, sizeof(buf), false));
    lua_push_int_table_entry(vm, "vlan", vlan_id);
    lua_push_int_table_entry(vm, "asn", asn);
    lua_push_str_table_entry(vm, "asname", asname);

    if(verbose) {
      lua_push_float_table_entry(vm, "latitude", latitude);
      lua_push_float_table_entry(vm, "longitude", longitude);
      lua_push_str_table_entry(vm, "country", country ? country : (char*)"");
      lua_push_str_table_entry(vm, "city", city ? city : (char*)"");
    }

    lua_push_int_table_entry(vm, "bytes.sent", sent.getNumBytes());
    lua_push_int_table_entry(vm, "bytes.rcvd", rcvd.getNumBytes());
    lua_push_int_table_entry(vm, "pkts.sent", sent.getNumPkts());
    lua_push_int_table_entry(vm, "pkts.rcvd", rcvd.getNumPkts());

    lua_push_int_table_entry(vm, "udp.pkts.sent",  udp_sent.getNumPkts());
    lua_push_int_table_entry(vm, "udp.bytes.sent", udp_sent.getNumBytes());
    lua_push_int_table_entry(vm, "udp.pkts.rcvd",  udp_rcvd.getNumPkts());
    lua_push_int_table_entry(vm, "udp.bytes.rcvd", udp_rcvd.getNumBytes());

    lua_push_int_table_entry(vm, "tcp.pkts.sent",  tcp_sent.getNumPkts());
    lua_push_int_table_entry(vm, "tcp.bytes.sent", tcp_sent.getNumBytes());
    lua_push_int_table_entry(vm, "tcp.pkts.rcvd",  tcp_rcvd.getNumPkts());
    lua_push_int_table_entry(vm, "tcp.bytes.rcvd", tcp_rcvd.getNumBytes());
 
    lua_push_int_table_entry(vm, "icmp.pkts.sent",  icmp_sent.getNumPkts());
    lua_push_int_table_entry(vm, "icmp.bytes.sent", icmp_sent.getNumBytes());
    lua_push_int_table_entry(vm, "icmp.pkts.rcvd",  icmp_rcvd.getNumPkts());
    lua_push_int_table_entry(vm, "icmp.bytes.rcvd", icmp_rcvd.getNumBytes());

    lua_push_int_table_entry(vm, "other_ip.pkts.sent",  other_ip_sent.getNumPkts());
    lua_push_int_table_entry(vm, "other_ip.bytes.sent", other_ip_sent.getNumBytes());
    lua_push_int_table_entry(vm, "other_ip.pkts.rcvd",  other_ip_rcvd.getNumPkts());
    lua_push_int_table_entry(vm, "other_ip.bytes.rcvd", other_ip_rcvd.getNumBytes());

    lua_push_int_table_entry(vm, "seen.first", first_seen);
    lua_push_int_table_entry(vm, "seen.last", last_seen);
    lua_push_int_table_entry(vm, "duration", get_duration());
    lua_push_str_table_entry(vm, "category", get_category());

    if(verbose) {
      if(ndpiStats) ndpiStats->lua(iface, vm);    
      if(localHost) getHostContacts(vm);
    }

    if(!returnHost) {
      lua_pushstring(vm, (ip != NULL) ? ip->print(buf, sizeof(buf)) : get_mac(buf, sizeof(buf)));
      lua_insert(vm, -2);
      lua_settable(vm, -3);
    }
  } else {
    lua_pushstring(vm,  get_name(buf, sizeof(buf), false));
    lua_pushinteger(vm, (lua_Integer)(sent.getNumBytes()+rcvd.getNumBytes()));
    lua_settable(vm, -3);
  }
}

/* ***************************************** */

/*
  As this method can be called from Lua, in order to avoid concurency issues
  we need to lock/unlock
*/
void Host::setName(char *name, bool update_categorization) {
  bool to_categorize = false;

  if(symbolic_name) return;

  m->lock(__FILE__, __LINE__);
  if((symbolic_name == NULL) || (symbolic_name && strcmp(symbolic_name, name))) {
    symbolic_name = strdup(name);
    to_categorize = true;
  }
  m->unlock(__FILE__, __LINE__);

  if(to_categorize && ntop->get_categorization())
    ntop->get_categorization()->findCategory(symbolic_name, category, sizeof(category), 
					     update_categorization);
}

/* ***************************************** */

void Host::refreshCategory() {
  if((symbolic_name != NULL) && (category[0] == '\0')) {
    ntop->get_categorization()->findCategory(symbolic_name, category, sizeof(category), false);
  }
}

/* ***************************************** */

char* Host::get_name(char *buf, u_int buf_len, bool force_resolution_if_not_found) {
  if(ip == NULL) {
    return(get_mac(buf, buf_len));
  } else {
    char *addr, redis_buf[64];
    int rc;

    if(symbolic_name != NULL)
      return(symbolic_name);

    addr = ip->print(buf, buf_len);
    rc = ntop->getRedis()->getAddress(addr, redis_buf, sizeof(redis_buf), 
				      force_resolution_if_not_found);

    if(rc == 0) {
      setName(redis_buf, false);
      return(symbolic_name);
    } else {
      symbolic_name = strdup(addr);
      return(addr);
    }
  }
}

/* *************************************** */

int Host::compare(Host *h) {
  if(ip)
    return(ip->compare(h->ip));
  else
    return(memcmp(mac_address, h->mac_address, 6));
}

/* ***************************************** */

bool Host::isIdle(u_int max_idleness) {
  return(((num_uses == 0) 
	  && (iface->getTimeLastPktRcvd() > (last_seen+max_idleness))) 
	 ? true : false);
}

/* ***************************************** */

bool Host::idle() {
  return(isIdle(ntop->getPrefs()->get_host_max_idle())); 
};

/* ***************************************** */

u_int32_t Host::key() { 
  if(ip)
    return(ip->key());    
  else {
    u_int32_t hash = 0;
    
    for(int i=0; i<6; i++) hash += mac_address[i] << (i+1);
    
    return(hash);
  }
}

/* *************************************** */

void Host::incrContact(Host *_peer, bool contacted_peer_as_client) {
  if(localHost && (_peer->get_ip() != NULL)) {
    char s_buf[32], d_buf[32], *me, *peer;

    me   = get_ip()->print(s_buf, sizeof(s_buf));
    peer = _peer->get_ip()->print(d_buf, sizeof(d_buf));

    ((GenericHost*)this)->incrContact(me, peer, contacted_peer_as_client);
  }
}

/* *************************************** */

void Host::incStats(u_int8_t l4_proto, u_int ndpi_proto, u_int64_t sent_packets, 
		    u_int64_t sent_bytes, u_int64_t rcvd_packets, u_int64_t rcvd_bytes) {
  if(sent_packets || rcvd_packets) {
    ((GenericHost*)this)->incStats(l4_proto, ndpi_proto, sent_packets,
				   sent_bytes, rcvd_packets, rcvd_bytes);

    switch(l4_proto) {
    case 0:
      /* Unknown protocol */
      break;
    case IPPROTO_UDP:
      udp_rcvd.incStats(rcvd_packets, rcvd_bytes),
	udp_sent.incStats(sent_packets, sent_bytes);
      break;
    case IPPROTO_TCP:
      tcp_rcvd.incStats(rcvd_packets, rcvd_bytes),
	tcp_sent.incStats(sent_packets, sent_bytes);
      break;
    case IPPROTO_ICMP:
      icmp_rcvd.incStats(rcvd_packets, rcvd_bytes), 
	icmp_sent.incStats(sent_packets, sent_bytes);
      break;
    default:
      other_ip_rcvd.incStats(rcvd_packets, rcvd_bytes),
	other_ip_sent.incStats(sent_packets, sent_bytes);
      break;
    }
  }
}
