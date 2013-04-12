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

/* **************************************** */

Prefs::Prefs(char *redis_host, int redis_port) {
  if(((redis = credis_connect(redis_host, redis_port, 10000)) == NULL)
     || (credis_ping(redis) != 0)) {
    printf("Unable to connect to redis %s:%d\n", redis_host, redis_port);
    exit(-1);
  }
}

/* **************************************** */

Prefs::~Prefs() {
  credis_close(redis);
}

/* **************************************** */

int Prefs::get(char *key, char *rsp, u_int rsp_len) {
  char *val;
  int rc;

  if((rc = credis_get(redis, key, &val)) == 0) {
    snprintf(rsp, rsp_len, "%s", val);
    free(val);
  } else
    rsp[0] = 0;

  return(rc);
}

/* **************************************** */

int Prefs::set(char *key, char *value) {
  return(credis_set(redis, key, value));
}

