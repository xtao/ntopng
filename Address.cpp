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
  ptree = New_Patricia(128);
}

/* *********************************************** */

static int fill_prefix_v4(prefix_t *p, struct in_addr *a, int b, int mb) {
  do {
    if(b < 0 || b > mb)
      return(-1);

    memcpy(&p->add.sin, a, (mb+7)/8);
    p->family = AF_INET;
    p->bitlen = b;
    p->ref_count = 0;
  } while (0);

  return(0);
}

/* ******************************************* */

static int fill_prefix_v6(prefix_t *prefix, struct in6_addr *addr, int bits, int maxbits) {
  if(bits < 0 || bits > maxbits)
    return -1;

  memcpy(&prefix->add.sin6, addr, (maxbits + 7) / 8);
  prefix->family = AF_INET6;
  prefix->bitlen = bits;
  prefix->ref_count = 0;

  return 0;
}

/* ******************************************* */

static patricia_node_t* add_to_ptree(patricia_tree_t *tree, int family, void *addr, int bits) {
  prefix_t prefix;
  patricia_node_t *node;

  if(family == AF_INET)
    fill_prefix_v4(&prefix, (struct in_addr*)addr, bits, tree->maxbits);
  else
    fill_prefix_v6(&prefix, (struct in6_addr*)addr, bits, tree->maxbits);

  node = patricia_lookup(tree, &prefix);

  return(node);
}

/* ******************************************* */

#if 0
static int remove_from_ptree(patricia_tree_t *tree, int family, void *addr, int bits) {
  prefix_t prefix;
  patricia_node_t *node;
  int rc;

  if(family == AF_INET)
    fill_prefix_v4(&prefix, (struct in_addr*)addr, bits, tree->maxbits);
  else
    fill_prefix_v6(&prefix, (struct in6_addr*)addr, bits, tree->maxbits);

  node = patricia_lookup(tree, &prefix);

  if((patricia_node_t *)0 != node) {
    rc = 0;
  } else {
    rc = -1;
  }

  return(rc);
}
#endif

/* ******************************************* */

static patricia_node_t* ptree_match(patricia_tree_t *tree, int family, void *addr, int bits) {
  prefix_t prefix;

  if(family == AF_INET)
    fill_prefix_v4(&prefix, (struct in_addr*)addr, bits, tree->maxbits);
  else
    fill_prefix_v6(&prefix, (struct in6_addr*)addr, bits, tree->maxbits);

  return(patricia_search_best(tree, &prefix));
}

/* ******************************************* */

static void ptree_add_rule(patricia_tree_t *ptree, char *line) {
  char *ip, *bits;
  struct in_addr addr4;
  struct in6_addr addr6;

  ip = line;
  bits  = strchr(line, '/');
  if (bits == NULL)
    return;

  bits[0] = '\0';  bits++;

  ntop->getTrace()->traceEvent(TRACE_DEBUG, "Rule '%s'/'%s'\n", ip, bits);

  if(strchr(ip, ':') != NULL) { /* IPv6 */
    if(inet_pton(AF_INET6, ip, &addr6) == 1)
      add_to_ptree(ptree, AF_INET6, &addr6, atoi(bits));
    else
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Error parsing IPv6 %s\n", ip);
  } else { /* IPv4 */
    /* inet_aton(ip, &addr4) fails parsing subnets */
    int num_octets, ip4_0 = 0, ip4_1 = 0, ip4_2 = 0, ip4_3 = 0;
    u_char *ip4 = (u_char *) &addr4;

    if((num_octets = sscanf(ip, "%u.%u.%u.%u", &ip4_0, &ip4_1, &ip4_2, &ip4_3)) >= 1) {
      ip4[0] = ip4_0, ip4[1] = ip4_1, ip4[2] = ip4_2, ip4[3] = ip4_3;

      if(num_octets * 8 < atoi(bits))
	ntop->getTrace()->traceEvent(TRACE_WARNING, "Found ip smaller than netmask\n");

      //addr4.s_addr = ntohl(addr4.s_addr);
      add_to_ptree(ptree, AF_INET, &addr4, atoi(bits));
    } else {
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Error parsing IPv4 %s\n", ip);
    }
  }
}

/* ******************************************* */

/* Format: 131.114.21.0/24,10.0.0.0/255.0.0.0 */
void Address::setLocalNetworks(char *rule) {
  char *net = strtok(rule, ",");

  while(net != NULL) {
    ptree_add_rule(ptree, net);

    net = strtok(NULL, ",");
  }
}

/* ******************************************* */

bool Address::findAddress(int family, void *addr) {
  return((ptree_match(ptree, family, addr, (family == AF_INET) ? 32 : 128) != NULL) ? true /* found */ : false /* not found */);
}

/* **************************************** */

static void free_ptree_data(void *data) { ; }

/* **************************************** */

Address::~Address() {
  void *res;

  pthread_join(resolveThreadLoop, &res);

  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Address resolution stats [%u resolved][%u failures]",
			       num_resolved_addresses, num_resolved_fails);

  if(ptree) Destroy_Patricia(ptree, free_ptree_data);
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

