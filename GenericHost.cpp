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

GenericHost::GenericHost(NetworkInterface *_iface) : GenericHashEntry(_iface) {
  ndpiStats = new NdpiStats();
  
  last_bytes = 0;
  bytes_thpt = 0, bytes_thpt_trend = trend_unknown;
  last_update_time.tv_sec = 0, last_update_time.tv_usec = 0;
}

/* *************************************** */

GenericHost::~GenericHost() {
  char buf[64], *keyname;

  keyname = get_string_key(buf, sizeof(buf));
  if(keyname[0] != '\0') {
    char key[64];

    snprintf(key, sizeof(key), "%s.client", keyname);
    ntop->getRedis()->del(key);

    snprintf(key, sizeof(key), "%s.server", keyname);
    ntop->getRedis()->del(key);
  }

  if(ndpiStats)
    delete ndpiStats;
}

/* *************************************** */

void GenericHost::incStats(u_int8_t l4_proto, u_int ndpi_proto,
			   u_int64_t sent_packets, u_int64_t sent_bytes,
			   u_int64_t rcvd_packets, u_int64_t rcvd_bytes) {
  if(sent_packets || rcvd_packets) {
    time_t when = iface->getTimeLastPktRcvd();
    
    /* Set a bit every CONST_TREND_TIME_GRANULARITY seconds */
    when -= when % CONST_TREND_TIME_GRANULARITY;
    activityStats.set(when);

    sent.incStats(sent_packets, sent_bytes), rcvd.incStats(rcvd_packets, rcvd_bytes);

    if((ndpi_proto != NO_NDPI_PROTOCOL) && ndpiStats)
      ndpiStats->incStats(ndpi_proto, sent_packets, sent_bytes, rcvd_packets, rcvd_bytes);
    
    updateSeen();
  }
}

/* *************************************** */

void GenericHost::updateStats(struct timeval *tv) {
  if(last_update_time.tv_sec > 0) {
    float tdiff = (tv->tv_sec-last_update_time.tv_sec)*1000+(tv->tv_usec-last_update_time.tv_usec)/1000;
    u_int64_t new_bytes = sent.getNumBytes()+rcvd.getNumBytes();

    tdiff = ((float)((new_bytes-last_bytes)*1000))/tdiff;

    if(bytes_thpt < tdiff)      bytes_thpt_trend = trend_up;
    else if(bytes_thpt > tdiff) bytes_thpt_trend = trend_down;
    else                        bytes_thpt_trend = trend_stable;

    bytes_thpt = tdiff, last_bytes = new_bytes;
  }

  memcpy(&last_update_time, tv, sizeof(struct timeval));
}
