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

/* ***************************************************** */

#define NTOP_SVN_REVISION         "$Revision$"
#define NTOP_SVN_REVISION_DATE    "$Date$"

#define NO_NDPI_PROTOCOL          ((u_int)-1)
#define GTP_U_V1_PORT             2152
#define MAX_NUM_INTERFACE_HOSTS   65536

#define FLOW_PURGE_FREQUENCY     1 /* sec */
#define HOST_PURGE_FREQUENCY     1 /* sec */

#define PURGE_FRACTION          32 /* check 1/32 of hashes per iteration */

#define DOMAIN_CATEGORY      "domain.category"
#define DOMAIN_TO_CATEGORIZE "domain.tocategorize"
#define DNS_CACHE            "dns.cache"
#define DNS_TO_RESOLVE       "dns.toresolve"

#ifndef MAX_PATH
#define MAX_PATH             256
#endif

#define CONST_DEFAULT_NTOP_PORT 3000

#ifdef WIN32
#define ntop_mkdir(a, b) _mkdir(a)
#define CONST_PATH_SEP                    '\\'
#else
#define ntop_mkdir(a, b) mkdir(a, b)
#define CONST_PATH_SEP                    '/'
#endif

#define CONST_DEFAULT_HOME_NET      "192.168.1.0/24"
#define CONST_DEFAULT_DATA_DIR      "data"
#define CONST_DEFAULT_DOCS_DIR      "httpdocs"
#define CONST_DEFAULT_SCRIPTS_DIR   "scripts"
#define CONST_DEFAULT_CALLBACKS_DIR "scripts/callbacks"
#define CONST_DEFAULT_USERS_FILE    "ntopng-users.conf"
#define CONST_DEFAULT_WRITABLE_DIR  "/var/tmp"
#define CONST_DEFAULT_INSTALL_DIR   "/usr/local/share/ntopng"
#define CONST_DEFAULT_NTOP_USER     "nobody"

#define PAGE_NOT_FOUND "<html><head><title>ntop</title></head><body><center><img src=/img/warning.png> Page &quot;%s&quot; was not found</body></html>"
#define PAGE_ERROR     "<html><head><title>ntop</title></head><body><img src=/img/warning.png> Script &quot;%s&quot; returned an error:<p>\n<pre>%s</pre></body></html>"
#define DENIED         "<html><head><title>Access denied</title></head><body>Access denied</body></html>"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifdef WIN32
// internal name of the service
#define SZSERVICENAME        "ntopng"

// displayed name of the service
#define SZSERVICEDISPLAYNAME "ntopng Win32"

  // Service TYPE Permissable values:
  //		SERVICE_AUTO_START
  //		SERVICE_DEMAND_START
  //		SERVICE_DISABLED
#define SERVICESTARTTYPE SERVICE_AUTO_START

//
// MessageId: EVENT_GENERIC_INFORMATION
//
// MessageText:
//
//  %1
//
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

#endif /* _NTOP_DEFINES_H_ */
