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

#ifndef _NTOP_DEFINES_H_
#define _NTOP_DEFINES_H_

#include "ntop_flow.h"

#define NUM_ROOTS 512

/* ***************************************************** */

#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#endif

#ifndef ETHERTYPE_IPV6
#define	ETHERTYPE_IPV6		0x86DD	/* IPv6 protocol */
#endif

#ifndef ETHERTYPE_MPLS
#define	ETHERTYPE_MPLS		0x8847	/* MPLS protocol */
#endif

#ifndef ETHERTYPE_MPLS_MULTI
#define ETHERTYPE_MPLS_MULTI	0x8848	/* MPLS multicast packet */
#endif

#ifndef ETHERTYPE_ARP
#define	ETHERTYPE_ARP		0x0806	/* Address Resolution Protocol */
#endif

/* BSD AF_ values. */
#define BSD_AF_INET             2
#define BSD_AF_INET6_BSD        24      /* OpenBSD (and probably NetBSD), BSD/OS */
#define BSD_AF_INET6_FREEBSD    28
#define BSD_AF_INET6_DARWIN     30

/* ***************************************************** */

#define LOGIN_URL               "/login.html"
#define AUTHORIZE_URL           "/authorize.html"
#define HTTP_SESSION_DURATION   3600

#define NO_NDPI_PROTOCOL          ((u_int)-1)
#define NDPI_MIN_NUM_PACKETS      10
#define GTP_U_V1_PORT             2152
#define MAX_NUM_INTERFACE_HOSTS   131072
#define MAX_NUM_INTERFACES        16

#define HOST_FAMILY_ID            ((u_int16_t)-1)
#define FLOW_PURGE_FREQUENCY      1 /* sec */
#define HOST_PURGE_FREQUENCY      1 /* sec */
#define MAX_TCP_FLOW_IDLE         3 /* sec */
#define PURGE_FRACTION           32 /* check 1/32 of hashes per iteration */
#define MAX_NUM_QUEUED_ADDRS    500 /* Maximum number of queued address for resolution */
#define MAX_NUM_QUEUED_CONTACTS 25000
#define DEFAULT_PID_PATH        "/var/tmp/ntopng.pid"
#define DOMAIN_CATEGORY         "domain.category"
#define DOMAIN_TO_CATEGORIZE    "domain.tocategorize"
#define DNS_CACHE               "dns.cache"
#define DNS_TO_RESOLVE          "dns.toresolve"
#define DNS_HASH_TO_RESOLVE     "dns.toresolvehash"

#ifndef TH_FIN
#define	TH_FIN	0x01
#endif
#ifndef TH_SYN
#define	TH_SYN	0x02
#endif
#ifndef TH_RST
#define	TH_RST	0x04
#endif
#ifndef TH_PUSH
#define	TH_PUSH	0x08
#endif
#ifndef TH_ACK
#define	TH_ACK	0x10
#endif
#ifndef TH_URG
#define	TH_URG	0x20
#endif

#define MAX_NUM_DEFINED_INTERFACES 16
#define MAX_NUM_DB_SPINS            5 /* sec */

#ifndef MAX_PATH
#define MAX_PATH                  256
#endif

//#define DEMO_WIN32              1
#define MAX_NUM_PACKETS           2000

#define MAX_NUM_HOST_CONTACTS     32
#define CONST_DEFAULT_NTOP_PORT   3000

#define CONST_NUM_OPEN_DB_CACHE   8
#define CONST_NUM_CONTACT_DBS     8

#ifdef WIN32
#define ntop_mkdir(a, b) _mkdir(a)
#define CONST_PATH_SEP                    '\\'
#else
#define ntop_mkdir(a, b) mkdir(a, b)
#define CONST_PATH_SEP                    '/'
#endif

#define NTOPNG_NDPI_OS_PROTO_ID     (NDPI_LAST_IMPLEMENTED_PROTOCOL+NDPI_MAX_NUM_CUSTOM_PROTOCOLS-2)
#define CONST_DEFAULT_HOME_NET      "192.168.1.0/24"
#define CONST_DEFAULT_DATA_DIR      "data"
#define CONST_DEFAULT_DOCS_DIR      "httpdocs"
#define CONST_DEFAULT_SCRIPTS_DIR   "scripts"
#define CONST_DEFAULT_CALLBACKS_DIR "scripts/callbacks"
#define CONST_DEFAULT_USERS_FILE    "ntopng-users.conf"
#define CONST_DEFAULT_WRITABLE_DIR  "/var/tmp"
#define CONST_DEFAULT_INSTALL_DIR   "/usr/local/share/ntopng"
#define CONST_DEFAULT_NTOP_USER     "nobody"
#define CONST_TOO_EARLY             "(Too Early)"

#define CONST_HTTP_CONN                "http.conn"
#define CONST_LUA_OK                   1
#define CONST_LUA_ERROR                0
#define CONST_LUA_PARAM_ERROR         -1

#define CONST_CONTACTED_BY            "contacted_by"
#define CONST_CONTACTS                "contacted_peers" /* Peers contacted by this host */

#define CONST_AGGREGATIONS            "aggregations"
#define CONST_HOST_CONTACTS           "host_contacts"

#define CONST_MAX_ACTIVITY_DURATION    86400 /* sec */
#define CONST_TREND_TIME_GRANULARITY   1 /* sec */
#define CONST_DEFAULT_PRIVATE_NETS     "192.168.0.0/16,172.16.0.0/12,10.0.0.0/8,127.0.0.0/8"
#define CONST_DEFAULT_LOCAL_NETS       "0.0.0.0/32,224.0.0.0/8,239.0.0.0/8,255.255.255.255/32,127.0.0.0/8"

#define PAGE_NOT_FOUND "<html><head><title>ntop</title></head><body><center><img src=/img/warning.png> Page &quot;%s&quot; was not found</body></html>"
#define PAGE_ERROR     "<html><head><title>ntop</title></head><body><img src=/img/warning.png> Script &quot;%s&quot; returned an error:<p>\n<pre>%s</pre></body></html>"
#define DENIED         "<html><head><title>Access denied</title></head><body>Access denied</body></html>"

#define DUMP_CONTACTS_ON_REDIS         1
#ifdef DUMP_CONTACTS_ON_REDIS
#define CONST_DB_DAY_FORMAT            "%y%m%d"
#else
#define CONST_DB_DAY_FORMAT            "%y/%m/%d"
#endif
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifdef WIN32

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// internal name of the service
#define SZSERVICENAME        "ntopng"

// displayed name of the service
#define SZSERVICEDISPLAYNAME "ntopng Win32"

  // Service TYPE Permissable values:
  //		SERVICE_AUTO_START
  //		SERVICE_DEMAND_START
  //		SERVICE_DISABLED
#define SERVICESTARTTYPE SERVICE_AUTO_START

#define EVENT_GENERIC_INFORMATION        0x40000001L

  // =========================================================
  // You should not need any changes below this line
  // =========================================================

  // Value name for app parameters
#define SZAPPPARAMS "AppParameters"

  // list of service dependencies - "dep1\0dep2\0\0"
  // If none, use ""
#define SZDEPENDENCIES ""
#endif

#ifndef min_val
#define min_val(a,b) ((a < b) ? a : b)
#endif

#ifndef max_val
#define max_val(a,b) ((a > b) ? a : b)
#endif

#define ifdot(a) ((a == '.') ? '_' : a)

#endif /* _NTOP_DEFINES_H_ */
