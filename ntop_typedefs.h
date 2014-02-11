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

#ifndef _NTOP_TYPEDEFS_H_
#define _NTOP_TYPEDEFS_H_

typedef enum {
  trend_unknown = 0,
  trend_up = 1,
  trend_down = 2,
  trend_stable = 3
} ValueTrend;

typedef enum {
  location_none = 0,
  location_local_only,
  location_remote_only,
  location_all,
} LocationPolicy;

typedef enum {
  alert_syn_flood = 0,
  alert_flow_flood,
  alert_threshold_exceeded
} AlertType;

typedef enum {
  alert_level_info = 0,
  alert_level_warning,
  alert_level_error,
} AlertLevel;

#endif /* _NTOP_TYPEDEFS_H_ */
