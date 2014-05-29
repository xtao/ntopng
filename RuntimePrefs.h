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

#ifndef _RUNTIME_PREFS_H_
#define _RUNTIME_PREFS_H_

#include "ntop_includes.h"


class RuntimePrefs {
 public:
  RuntimePrefs();

  void set_local_hosts_rrd_creation(bool enable);
  bool are_local_hosts_rrd_created();
  void set_hosts_ndpi_rrd_creation(bool enable);
  bool are_hosts_ndpi_rrd_created();
};

#endif /* _RUNTIME_PREFS_H_ */
