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

Redis::Redis(char *redis_host, int redis_port) {
  if(((redis = credis_connect(redis_host, redis_port, 10000)) == NULL)
     || (credis_ping(redis) != 0)) {
    printf("Unable to connect to redis %s:%d\n", redis_host, redis_port);
    exit(-1);
  }

  l = new Mutex();
  setDefaults();
}

/* **************************************** */

Redis::~Redis() {
  credis_close(redis);
  delete l;
}

/* **************************************** */

int Redis::expire(char *key, u_int expire_sec) {
  int rc;

  l->lock(__FILE__, __LINE__);
  rc = credis_expire(redis, key, expire_sec);
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::get(char *key, char *rsp, u_int rsp_len) {
  char *val;
  int rc;

  l->lock(__FILE__, __LINE__);
  if((rc = credis_get(redis, key, &val)) == 0) {
    snprintf(rsp, rsp_len, "%s", val);
    // free(val);
  } else
    rsp[0] = 0;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::set(char *key, char *value, u_int expire_secs) {
  int rc;

  l->lock(__FILE__, __LINE__);
  rc = credis_set(redis, key, value);

  if((rc == 0) && (expire_secs != 0))
    credis_expire(redis, key, expire_secs);
  l->unlock(__FILE__, __LINE__);

  // ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s <-> %s", key, value);
  return(rc);
}

/* **************************************** */

int Redis::queueHostToResolve(char *hostname) {
  if(ntop->getPrefs()->is_dns_resolution_enabled()) {
    int rc;
    char key[128], *val;

    l->lock(__FILE__, __LINE__);

    snprintf(key, sizeof(key), "dns.cache.%s", hostname);
    /*
      Add only if the address has not been resolved yet
    */
    if(credis_get(redis, key, &val) < 0)
      rc = credis_rpush(redis, "dns.toresolve", hostname);

    l->unlock(__FILE__, __LINE__);

    return(rc);
  } else
    return(0);
}

/* **************************************** */

int Redis::popHostToResolve(char *hostname, u_int hostname_len) {
  char *val;
  int rc;

  l->lock(__FILE__, __LINE__);
  rc = credis_lpop(redis, (char*)"dns.toresolve", &val);
  l->unlock(__FILE__, __LINE__);

  if(rc == 0)
    snprintf(hostname, hostname_len, "%s", val);
  else
    hostname[0] = '\0';

  return(rc);
}

/* **************************************** */

void Redis::setDefaults() {
  setResolvedAddress((char*)"127.0.0.1", (char*)"localhost");
  setResolvedAddress((char*)"255.255.255.255", (char*)"Broadcast");
  setResolvedAddress((char*)"0.0.0.0", (char*)"No IP");
}

/* **************************************** */

int Redis::getAddress(char *numeric_ip, char *rsp, u_int rsp_len) {
  char key[64];
  int rc;

  snprintf(key, sizeof(key), "dns.cache.%s", numeric_ip);
  rc = get(key, rsp, rsp_len);

  if(rc != 0)
    queueHostToResolve(numeric_ip);
  else {
    /* We need to extend expire */

    expire(numeric_ip, 300 /* expire */);
  }

  return(rc);
}

/* **************************************** */

int Redis::setResolvedAddress(char *numeric_ip, char *symbolic_ip) {
  char key[64];

  snprintf(key, sizeof(key), "dns.cache.%s", numeric_ip);
  return(set(key, symbolic_ip, 300));
}
