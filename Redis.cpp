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

#include "third-party/hiredis/hiredis.c"
#include "third-party/hiredis/net.c"
#include "third-party/hiredis/sds.c"

/* **************************************** */

Redis::Redis(char *redis_host, int redis_port) {
  struct timeval timeout = { 1, 500000 }; // 1.5 seconds
  redisReply *reply;

  redis = redisConnectWithTimeout(redis_host, redis_port, timeout);

  if(redis) reply = (redisReply*)redisCommand(redis, "PING"); else reply = NULL;
  
  if((redis == NULL) || (reply == NULL)) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "ntopng requires redis server to be up and running");
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Please start it and try again or use -r");
    ntop->getTrace()->traceEvent(TRACE_ERROR, "to specify a redis server other than the default");
    exit(-1);
  } else
    freeReplyObject(reply);

  ntop->getTrace()->traceEvent(TRACE_NORMAL,
			       "Successfully connected to Redis %s:%u",
			       redis_host, redis_port);
  l = new Mutex();
  setDefaults();
}

/* **************************************** */

Redis::~Redis() {
  redisFree(redis);
  delete l;
}

/* **************************************** */

int Redis::expire(char *key, u_int expire_sec) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "EXPIRE %s %u", key, expire_sec);
  l->unlock(__FILE__, __LINE__);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;

  return(rc);
}

/* **************************************** */

int Redis::get(char *key, char *rsp, u_int rsp_len) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "GET %s", key);

  if(reply && reply->str) {
    snprintf(rsp, rsp_len, "%s", reply->str), rc = 0;
  } else
    rsp[0] = 0, rc = -1;
  l->unlock(__FILE__, __LINE__);

  if(reply) freeReplyObject(reply);

  return(rc);
}

/* **************************************** */

int Redis::hashGet(char *key, char *field, char *rsp, u_int rsp_len) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "HGET %s %s", key, field);

  if(reply && reply->str) {
    snprintf(rsp, rsp_len, "%s", reply->str), rc = 0;
  } else
    rsp[0] = 0, rc = -1;
  l->unlock(__FILE__, __LINE__);

  if(reply) freeReplyObject(reply);

  return(rc);
}

/* **************************************** */

int Redis::set(char *key, char *value, u_int expire_secs) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "SET %s %s", key, value);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;

  if((rc == 0) && (expire_secs != 0)) {
    reply = (redisReply*)redisCommand(redis, "EXPIRE %s %u", key, expire_secs);
    if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  }
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

char* Redis::popSet(char *pop_name, char *rsp, u_int rsp_len) {
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "SPOP %s", pop_name);

  if(reply && reply->str) {
    snprintf(rsp, rsp_len, "%s", reply->str);
  } else
    rsp[0] = 0;
  l->unlock(__FILE__, __LINE__);

  if(reply) freeReplyObject(reply);

  return(rsp);
}

/* **************************************** */

/*
  Incrememnt key.member of +value and keeps at most trim_len elements
*/
int Redis::zincrbyAndTrim(char *key, char *member, u_int value, u_int trim_len) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "ZINCRBY %s %u", key, value);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;

  if((rc == 0) && (trim_len > 0)) {
    reply = (redisReply*)redisCommand(redis, "ZREMRANGEBYRANK %s 0 %u", key, -1*trim_len);
    if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  }
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::keys(const char *pattern, char ***keys_p) {
  int rc = 0;
  u_int i;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "KEYS %s", pattern);
  if(reply->type == REDIS_REPLY_ARRAY) {
    (*keys_p) = (char**) malloc(reply->elements * sizeof(char*));
    rc = reply->elements;

    for(i = 0; i < reply->elements; i++) {
      (*keys_p)[i] = strdup(reply->element[i]->str);
    }
  }

  if(reply) freeReplyObject(reply);
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::hashKeys(const char *pattern, char ***keys_p) {
  int rc = 0;
  u_int i;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "HKEYS %s", pattern);
  if(reply->type == REDIS_REPLY_ARRAY) {
    (*keys_p) = (char**) malloc(reply->elements * sizeof(char*));
    rc = reply->elements;
    
    for(i = 0; i < reply->elements; i++) {
      (*keys_p)[i] = strdup(reply->element[i]->str);
    }
  }

  if(reply) freeReplyObject(reply);
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::del(char *key) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "DEL %s", key);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::delHash(char *key) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "HDEL %s", key);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::queueHostToResolve(char *hostname, bool dont_check_for_existance, bool localHost) {
  int rc;
  char key[128];
  bool found;
  redisReply *reply;

  if(!ntop->getPrefs()->is_dns_resolution_enabled()) return(0);

  snprintf(key, sizeof(key), "%s.%s", DNS_CACHE, hostname);

  l->lock(__FILE__, __LINE__);

  if(dont_check_for_existance)
    found = false;
  else {
    /*
      Add only if the address has not been resolved yet
    */

    reply = (redisReply*)redisCommand(redis, "GET %s", key);
    if(reply && reply->str)
      found = false;
    else
      found = true;

    if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  }

  if(!found) {
    /* Add to the list of addresses to resolve */

    reply = (redisReply*)redisCommand(redis, "%s %s %s",
			 localHost ? "LPUSH" : "RPUSH",
			 DNS_TO_RESOLVE, hostname);
    if(reply) freeReplyObject(reply), rc = 0; else rc = -1;

    /*
      We make sure that no more than MAX_NUM_QUEUED_ADDRS entries are in queue
      This is important in order to avoid the cache to grow too much
    */
    reply = (redisReply*)redisCommand(redis, "LTRIM %s 0 %u", DNS_TO_RESOLVE, MAX_NUM_QUEUED_ADDRS);
    if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  } else
    reply = 0;

  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

int Redis::popHostToResolve(char *hostname, u_int hostname_len) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "LPOP %s", DNS_TO_RESOLVE);

  if(reply && reply->str)
    snprintf(hostname, hostname_len, "%s", reply->str), rc = 0;
  else
    hostname[0] = '\0', rc = -1;

  if(reply) freeReplyObject(reply);
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

char* Redis::getFlowCategory(char *domainname, char *buf,
			     u_int buf_len, bool categorize_if_unknown) {
  char key[128];
  redisReply *reply;

  buf[0] = 0;

  if(!ntop->getPrefs()->is_categorization_enabled())  return(NULL);

  /* Check if the host is 'categorizable' */
  if(Utils::isIPAddress(domainname)) {
    return(buf);
  }

  l->lock(__FILE__, __LINE__);

  snprintf(key, sizeof(key), "%s.%s", DOMAIN_CATEGORY, domainname);

  /*
    Add only if the domain has not been categorized yet
  */
  reply = (redisReply*)redisCommand(redis, "GET %s", key);

  if(reply && reply->str) {
    snprintf(buf, buf_len, "%s", reply->str);
    if(reply) freeReplyObject(reply);
  } else {
    buf[0] = 0;

    if(categorize_if_unknown) {
      reply = (redisReply*)redisCommand(redis, "RPUSH %s %s", DOMAIN_TO_CATEGORIZE, domainname);
      if(reply) freeReplyObject(reply);
    }
  }

  l->unlock(__FILE__, __LINE__);

  return(buf);
}

/* **************************************** */

int Redis::popDomainToCategorize(char *domainname, u_int domainname_len) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "LPOP %s", DOMAIN_TO_CATEGORIZE);

  if(reply && reply->str)
    snprintf(domainname, domainname_len, "%s", reply->str);
  else
    domainname[0] = '\0';

  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

void Redis::setDefaults() {
  char value[32];

  setResolvedAddress((char*)"127.0.0.1", (char*)"localhost");
  setResolvedAddress((char*)"255.255.255.255", (char*)"Broadcast");
  setResolvedAddress((char*)"0.0.0.0", (char*)"NoIP");

  if(get((char*)"user.admin.password", value, sizeof(value)) < 0)
    set((char*)"user.admin.password", (char*)"21232f297a57a5a743894a0e4a801fc3");
}

/* **************************************** */

int Redis::getAddress(char *numeric_ip, char *rsp,
		      u_int rsp_len, bool queue_if_not_found) {
  char key[64];
  int rc;

  rsp[0] = '\0';
  snprintf(key, sizeof(key), "%s.%s", DNS_CACHE, numeric_ip);

  rc = get(key, rsp, rsp_len);

  if(rc != 0) {
    if(queue_if_not_found)
      queueHostToResolve(numeric_ip, true, false);
  } else {
    /* We need to extend expire */

    expire(numeric_ip, 300 /* expire */);
  }

  return(rc);
}

/* **************************************** */

int Redis::setResolvedAddress(char *numeric_ip, char *symbolic_ip) {
  char key[64];

  snprintf(key, sizeof(key), "%s.%s", DNS_CACHE, numeric_ip);
  return(set(key, symbolic_ip, 300));
}

/* **************************************** */

char* Redis::getVersion(char *str, u_int str_len) {
#if 1
  snprintf(str, str_len, "%s" , "????");
#else
  // TODO
  REDIS_INFO info;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  if(redis && (credis_info(redis, &info) == 0))
    snprintf(str, str_len, "%s (%d bit)", info.redis_version, info.arch_bits);
  else
    str[0] = 0;
  l->unlock(__FILE__, __LINE__);

  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
#endif
  return(str);
}

/* **************************************** */

void Redis::getHostContacts(lua_State* vm, GenericHost *h, bool client_contacts) {
  char hkey[64], key[64];
  redisReply *reply;

  h->get_string_key(hkey, sizeof(hkey));
  if(hkey[0] == '\0') return;

  snprintf(key, sizeof(key), "%s.%s", hkey,
	   client_contacts ? "client" : "server");

  lua_newtable(vm);

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, 
				    "ZREVRANGE %s %u %u WITHSCORES", 
				    key, 0, -1);

  if(reply->type == REDIS_REPLY_ARRAY) {
    for(u_int i=0; i<(reply->elements-1); i++) {
      if((i % 2) == 0) {
	const char *key = (const char*)reply->element[i]->str;
	u_int64_t value = (u_int64_t)atol(reply->element[i+1]->str);

	//ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s:%llu", key, value);
	lua_push_int_table_entry(vm, key, value);
      }
    }
  }

  if(reply) freeReplyObject(reply);
  l->unlock(__FILE__, __LINE__);
}

/* **************************************** */

int Redis::incrContact(char *key, u_int16_t family_id,
		       char *peer, u_int32_t value) {
  int rc;
  redisReply *reply;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "HINCRBY %s %s@%u %u ",
				    key, peer, family_id, value);
  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}

/* **************************************** */

/*
  Hosts:       [name == NULL] && [ip != NULL]
  StringHosts: [name != NULL] && [ip == NULL]
*/
int Redis::addIpToDBDump(NetworkInterface *iface, IpAddress *ip, char *name) {
  char buf[64], daybuf[32], *what;
  int rc;
  redisReply *reply;
  time_t when = time(NULL);

  strftime(daybuf, sizeof(daybuf), CONST_DB_DAY_FORMAT, localtime(&when));
  what = ip ? ip->print(buf, sizeof(buf)) : name;

  l->lock(__FILE__, __LINE__);
  reply = (redisReply*)redisCommand(redis, "SADD %s.keys %s|%s|%s", daybuf,
				    ip ? CONST_HOST_CONTACTS : CONST_AGGREGATIONS,
				    iface->get_name(), what);

  ntop->getTrace()->traceEvent(TRACE_INFO, "Dumping %s", what);

  if(reply) freeReplyObject(reply), rc = 0; else rc = -1;
  l->unlock(__FILE__, __LINE__);

  return(rc);
}
