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

#ifndef _UTILS_H_
#define _UTILS_H_

#include "ntop_includes.h"

/* ******************************* */

class Utils {
 private:

 public:
  static char* formatTraffic(float numBits, bool bits, char *buf);
  static char* formatPackets(float numPkts, char *buf);
  static char* l4proto2name(u_int8_t proto);
  static bool  isIPAddress(char *name);
  static void  setThreadAffinity(pthread_t thread, int core_id);
  static char* trim(char *s);
  static u_int32_t hashString(char *s);
  static float timeval2ms(struct timeval *tv);
  static bool mkdir_tree(char *path);
  static void dropPrivileges();
};


#endif /* _UTILS_H_ */
