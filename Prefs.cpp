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

/* ******************************************* */

Prefs::Prefs() {
  enable_dns_resolution = sniff_dns_responses = true;
  categorization_enabled = false, resolve_all_host_ip = false;
  host_max_idle = 60 /* sec */, flow_max_idle = 30 /* sec */;
  max_num_hosts = 32768, max_num_flows = 65536;
}

/* ******************************************* */

Prefs::~Prefs() {
  ;
}

