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

#ifndef _REDIS_H_
#define _REDIS_H_

#include "ntop_includes.h"

class Host;

class Redis {
  redisContext *redis;
  Mutex *l;

  void setDefaults();

 public:
  Redis(char *redis_host = (char*)"127.0.0.1", int redis_port = 6379);
  ~Redis();

  char* getVersion(char *str, u_int str_len);

  int expire(char *key, u_int expire_sec);
  int get(char *key, char *rsp, u_int rsp_len);
  int hashGet(char *key, char *member, char *rsp, u_int rsp_len);
  int hashDel(char *key, char *field);
  int set(char *key, char *value, u_int expire_secs=0);
  char* popSet(char *pop_name, char *rsp, u_int rsp_len);
  int keys(const char *pattern, char ***keys_p);
  int hashKeys(const char *pattern, char ***keys_p);
  int del(char *key); 
  int delHash(char *key, char *member); 
  int zincrbyAndTrim(char *key, char *member, u_int value, u_int trim_len);

  int queueHostToResolve(char *hostname, bool dont_check_for_existance, bool localHost);
  int popHostToResolve(char *hostname, u_int hostname_len);

  char* getFlowCategory(char *domainname, char *buf, u_int buf_len, bool categorize_if_unknown);
  int popDomainToCategorize(char *domainname, u_int domainname_len);
  
  int getAddress(char *numeric_ip, char *rsp, u_int rsp_len, bool queue_if_not_found);
  int setResolvedAddress(char *numeric_ip, char *symbolic_ip);

  void getHostContacts(lua_State* vm, GenericHost *h, bool client_contacts);
  int incrContact(char *key, u_int16_t family_id, char *peer, u_int32_t value);

  int addIpToDBDump(NetworkInterface *iface, IpAddress *ip, char *name);

  int smembers(lua_State* vm, char *setName);

  bool dumpDailyStatsKeys(char *day);
};

#endif /* _REDIS_H_ */
