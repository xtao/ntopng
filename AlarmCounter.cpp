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

AlarmCounter::AlarmCounter(u_int16_t _max_num_hits_sec) {
  time_last_hit = time_last_alarm_reported = 0;
  num_hits_rcvd_last_second = 0;
  max_num_hits_sec = _max_num_hits_sec;
}

/* *************************************** */

bool AlarmCounter::incHits(time_t when) {
  if(time_last_hit != when) 
    time_last_hit = when, num_hits_rcvd_last_second = 1;
  else {
    num_hits_rcvd_last_second++;
    
    if(num_hits_rcvd_last_second > max_num_hits_sec) {
      if(when > (time_last_alarm_reported+CONST_ALARM_GRACE_PERIOD)) {
	time_last_alarm_reported = when;
	return(true);
      }
    }
  }

  return(false);
}
