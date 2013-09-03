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

#ifndef _TREND_STATS_H_
#define _TREND_STATS_H_

#include "ntop_includes.h"

/*
  Statistics for 1 day (86400 sec) 
*/

class TrendStats {
 private:
  time_t begin_time, end_time;
  void *_bitset;
    
 public:
  TrendStats(time_t _begin_time, u_int _duration_sec);
  ~TrendStats();

  void reset();
  void set(time_t when);
  std::stringstream* getDump();
  void setDump(std::stringstream* dump);
  void print();
};

#endif /* _TREND_STATS_H_ */
