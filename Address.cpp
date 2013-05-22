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

Address::Address() {
  num_resolved_addresses = num_resolved_fails = 0;
}

/* **************************************** */

Address::~Address() {
  void *res;

  pthread_join(resolveThreadLoop, &res);

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Address resolution stats [%u resolved][%u failures]",
			       num_resolved_addresses, num_resolved_fails);
}

/* ***************************************** */

void Address::resolveHostName(char *numeric_ip) {
  char rsp[128];
  
  if(ntop->getRedis()->getAddress(numeric_ip, rsp, sizeof(rsp)) < 0) {
    char hostname[NI_MAXHOST];
    struct sockaddr *sa;
    struct sockaddr_in in4;
    struct sockaddr_in6 in6;
    int rc, len;

    if(strchr(numeric_ip, '.') != NULL) {
      in4.sin_family = AF_INET, in4.sin_addr.s_addr = inet_addr(numeric_ip);
      len = sizeof(struct sockaddr_in), sa = (struct sockaddr*)&in4;
    } else {
      memset(&in6, 0, sizeof(struct sockaddr_in6));

      in6.sin6_family = AF_INET6, inet_pton(AF_INET6, numeric_ip, &in6.sin6_addr);
      len = sizeof(struct sockaddr_in6), sa = (struct sockaddr*)&in6;
    }

    if((rc = getnameinfo(sa, len, hostname, sizeof(hostname), NULL, 0, NI_NAMEREQD)) == 0) {
      ntop->getRedis()->setResolvedAddress(numeric_ip, hostname);
      num_resolved_addresses++;
    } else {
      num_resolved_fails++;
      ntop->getTrace()->traceEvent(TRACE_INFO, "Error resolution failure for %s [%d/%s/%s]", 
				   numeric_ip, rc, gai_strerror(rc), strerror(errno));
      ntop->getRedis()->setResolvedAddress(numeric_ip, numeric_ip); /* So we avoid to continuously resolver the same address */
    }
  }
}

/* **************************************************** */

static void* resolveLoop(void* ptr) {
  Address *a = (Address*)ptr;
  Redis *r = ntop->getRedis();

  while(!ntop->getGlobals()->isShutdown()) {
    char numeric_ip[64];
    int rc = r->popHostToResolve(numeric_ip, sizeof(numeric_ip));

    if(rc == 0) {
      a->resolveHostName(numeric_ip);
    } else
      sleep(1);
  }

  return(NULL);
}

/* **************************************************** */

void Address::startResolveAddressLoop() {
  pthread_create(&resolveThreadLoop, NULL, resolveLoop, (void*)this);
}
