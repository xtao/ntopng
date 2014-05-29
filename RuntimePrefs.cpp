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

/* ******************************************* */

RuntimePrefs::RuntimePrefs() {
  /* Force preferences creation */
  are_local_hosts_rrd_created();
  are_hosts_ndpi_rrd_created();
}

/* ******************************************* */

void RuntimePrefs::set_local_hosts_rrd_creation(bool enable) {
  ntop->getRedis()->set((char*)CONST_RUNTIME_PREFS_HOST_RRD_CREATION, 
			enable ? (char*)"1" : (char*)"0", 0);
}

/* ******************************************* */

bool RuntimePrefs::are_local_hosts_rrd_created() {
  char rsp[32];
  
  if(ntop->getRedis()->get((char*)CONST_RUNTIME_PREFS_HOST_RRD_CREATION, 
			   rsp, sizeof(rsp)) < 0) {
    set_local_hosts_rrd_creation(true);
    return(true);
  } else
    return((strcmp(rsp, "1") == 0) ? true : false);
}

/* ******************************************* */

void RuntimePrefs::set_hosts_ndpi_rrd_creation(bool enable) {
  ntop->getRedis()->set((char*)CONST_RUNTIME_PREFS_HOST_NDPI_RRD_CREATION, 
			enable ? (char*)"1" : (char*)"0", 0);
}

/* ******************************************* */

bool RuntimePrefs::are_hosts_ndpi_rrd_created() {
  char rsp[32];
  
  if(ntop->getRedis()->get((char*)CONST_RUNTIME_PREFS_HOST_NDPI_RRD_CREATION, 
			   rsp, sizeof(rsp)) < 0) {
    set_hosts_ndpi_rrd_creation(true);
    return(true);
  } else
    return((strcmp(rsp, "1") == 0) ? true : false);
}
