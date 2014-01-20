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

#ifndef _ALARM_COUNTER_H_
#define _ALARM_COUNTER_H_

#include "ntop_includes.h"

/** @class AlarmCounter
 *  @brief Base class for alarms.
 *  @details Defines a basic class for handling generated alarms.
 *
 *  @ingroup MonitoringData
 *
 */

class AlarmCounter {
 private:
  u_int16_t max_num_hits_sec; /**< Threshold above which we trigger an alarm. */
  time_t time_last_hit; /**< Time of last hit received. */ 
  time_t time_last_alarm_reported; /**< Time of last alarm issued. */ 
  u_int16_t num_hits_rcvd_last_second; /**< Number of hits reported in the last second. */ 

 public:
  AlarmCounter(u_int16_t _max_num_hits_sec);
  
  bool incHits(time_t when);
};

#endif /* _ALARM_COUNTER_H_ */
