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

AlertCounter::AlertCounter(u_int16_t _max_num_hits_sec,
			   u_int8_t _over_threshold_duration_sec) {
  time_last_hit = time_last_alert_reported = 0;
  num_hits_rcvd_last_second = 0;
  last_trepassed_threshold = 0, num_trepassed_threshold = 0;
  max_num_hits_sec = _max_num_hits_sec;
  over_threshold_duration_sec = _over_threshold_duration_sec;
  if(over_threshold_duration_sec < 1) over_threshold_duration_sec = 1;
}

/* *************************************** */

bool AlertCounter::incHits(time_t when) {
  if(time_last_hit != when)
    time_last_hit = when, num_hits_rcvd_last_second = 1;
  else {
    num_hits_rcvd_last_second++;

    if((num_hits_rcvd_last_second > max_num_hits_sec)
       && (last_trepassed_threshold != when)) {
      if((last_trepassed_threshold > 0) || (last_trepassed_threshold == (when-1)))
	num_trepassed_threshold++;
      else
	num_trepassed_threshold = 1;

      ntop->getTrace()->traceEvent(TRACE_NORMAL, "Trepass [num: %u][last: %u][now: %u][duration: %u][%p]",
				   num_trepassed_threshold, last_trepassed_threshold, when,
				   over_threshold_duration_sec, this);

      last_trepassed_threshold = when;
      
      if(num_trepassed_threshold > over_threshold_duration_sec) {
	if(when > (time_last_alert_reported+CONST_ALERT_GRACE_PERIOD)) {
	  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Alert emitted [num: %u][now: %u][duration: %u][%p]",
				       num_trepassed_threshold, when, over_threshold_duration_sec, this);
	  time_last_alert_reported = when;
	  return(true);
	}
      }
    }
  }

  return(false);
}
