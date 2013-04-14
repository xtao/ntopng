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

bool Address::isNumericIp(char *numeric_ip) {
  int a, b, c, d;

  /* FIX: Add IPv6 support */
  if(sscanf(numeric_ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    return(true);

  return(false);
}

/* ***************************************** */

void Address::resolveHostName(char *numeric_ip) {
  if(isNumericIp(numeric_ip)) {
    char rsp[128];

    if(ntop->getRedis()->getAddress(numeric_ip, rsp, sizeof(rsp)) < 0) {
      char hostname[NI_MAXHOST], sbuf[NI_MAXSERV];
      struct sockaddr sa;
      struct sockaddr_in *in4 = (struct sockaddr_in*)&sa;

      /* FIX - Add IPv6 support */
      in4->sin_family = AF_INET, in4->sin_addr.s_addr = inet_addr(numeric_ip);

      if(getnameinfo(&sa, sizeof(sa), hostname, NI_MAXHOST, sbuf, NI_MAXSERV, NI_NAMEREQD) == 0) {
	ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s -> %s", numeric_ip, hostname);
	ntop->getRedis()->setResolvedAddress(numeric_ip, hostname);
	num_resolved_addresses++;
      } else {
	num_resolved_fails++;
	ntop->getRedis()->setResolvedAddress(numeric_ip, numeric_ip); /* So we avoid to continuously resolver the same address */
      }
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
