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

IpAddress::IpAddress() {
  ;
}

/* ******************************************* */

IpAddress::IpAddress(u_int32_t _ipv4) {
  set_ipv4(_ipv4);
}

/* ******************************************* */

IpAddress::IpAddress(struct in6_addr _ipv6) {
  set_ipv6(_ipv6);
}

/* ******************************************* */

int IpAddress::compare(IpAddress *ip) {
  if(addr.ipVersion < ip->addr.ipVersion) return(-1); else if(addr.ipVersion > ip->addr.ipVersion) return(1);

  if(addr.ipVersion == 4)
    return(memcmp(&addr.ipType.ipv4, &ip->addr.ipType.ipv4, sizeof(u_int32_t)));
  else
    return(memcmp(&addr.ipType.ipv6, &ip->addr.ipType.ipv6, sizeof(struct in6_addr)));
}

/* ******************************************* */

u_int32_t IpAddress::key() {
  if(addr.ipVersion == 4)
    return(addr.ipType.ipv4);
  else {
    u_int32_t key=0;

    for(u_int32_t i=0; i<16; i++)
      key += addr.ipType.ipv6.s6_addr[i];

    return(key);
  }
}

/* ******************************************* */

char* IpAddress::_intoaV4(unsigned int addr, char* buf, u_short bufLen) {
  char *cp, *retStr;
  uint byte;
  int n;

  cp = &buf[bufLen];
  *--cp = '\0';

  n = 4;
  do {
    byte = addr & 0xff;
    *--cp = byte % 10 + '0';
    byte /= 10;
    if(byte > 0) {
      *--cp = byte % 10 + '0';
      byte /= 10;
      if(byte > 0)
	*--cp = byte + '0';
    }
    *--cp = '.';
    addr >>= 8;
  } while (--n > 0);

  /* Convert the string to lowercase */
  retStr = (char*)(cp+1);

  return(retStr);
}

/* ****************************** */

char* IpAddress::_intoa(char* buf, u_short bufLen) {
  if((addr.ipVersion == 4) || (addr.ipVersion == 0 /* Misconfigured */))
    return(_intoaV4(ntohl(addr.ipType.ipv4), buf, bufLen));
  else {
    char *ret = (char*)inet_ntop(AF_INET6, &addr.ipType.ipv6, buf, bufLen);

    if(ret == NULL) {
      /* Internal error (buffer too short) */
      buf[0] = '\0';
    }

    return(buf);
  }
}

/* ******************************************* */

char* IpAddress::print(char *str, u_int str_len) {
  return(_intoa(str, str_len));
}
