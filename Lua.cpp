/*
 *
 * (C) 2013-14 - ntop.org
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

#ifndef _GETOPT_H
#define _GETOPT_H
#endif

#ifndef LIB_VERSION
#define LIB_VERSION "1.4.7"
#endif

extern "C" {
#include "rrd.h"
#ifdef HAVE_GEOIP
  extern const char * GeoIP_lib_version(void);
#endif

#include "third-party/snmp/snmp.c"
#include "third-party/snmp/asn1.c"
#include "third-party/snmp/net.c"
};

#include "third-party/lsqlite3/lsqlite3.c"


/* Included by Categorization.cpp */
extern char* http_get(char *host, u_int port, char *page, char* rsp, u_int rsp_len);

/* ******************************* */

Lua::Lua() {
  L = luaL_newstate();
}

/* ******************************* */

Lua::~Lua() {
  lua_close(L);
}

/* ******************************* */

/**
 * @brief Check the expected type of lua function.
 * @details Find in the lua stack the function and check the function parameters types.
 *
 * @param vm The lua state.
 * @param func The function name.
 * @param pos Index of lua stack.
 * @param expected_type Index of expected type.
 * @return @ref CONST_LUA_ERROR if the expected type is equal to function type, @ref CONST_LUA_PARAM_ERROR otherwise.
 */
static int ntop_lua_check(lua_State* vm, const char* func,
			  int pos, int expected_type) {
  if(lua_type(vm, pos) != expected_type) {
    ntop->getTrace()->traceEvent(TRACE_ERROR,
				 "%s : expected %s, got %s", func,
				 lua_typename(vm, expected_type),
				 lua_typename(vm, lua_type(vm,pos)));
    return(CONST_LUA_PARAM_ERROR);
  }

  return(CONST_LUA_ERROR);
}

/* ****************************************** */

static NetworkInterface* handle_null_interface(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "Null interface: did you restart ntopng in the meantime?");

  return(ntop->getInterfaceAtId(0));
}

/* ****************************************** */

static int ntop_dump_file(lua_State* vm) {
  char *fname;
  FILE *fd;
  struct mg_connection *conn;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, CONST_HTTP_CONN);
  if((conn = (struct mg_connection*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null HTTP connection");
    return(CONST_LUA_ERROR);
  }

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((fname = (char*)lua_tostring(vm, 1)) == NULL)     return(CONST_LUA_PARAM_ERROR);

  ntop->fixPath(fname);
  if((fd = fopen(fname, "r")) != NULL) {
    char tmp[1024];

    ntop->getTrace()->traceEvent(TRACE_INFO, "[HTTP] Serving file %s", fname);

    while((fgets(tmp, sizeof(tmp), fd)) != NULL) {
      char *http_prefix = strstr(tmp, CONST_HTTP_PREFIX_STRING);

      if(http_prefix == NULL) {
	mg_printf(conn, "%s", tmp);
      } else {
	char *begin = tmp;
	int len = strlen(CONST_HTTP_PREFIX_STRING);

	/* Replace CONST_HTTP_PREFIX_STRING with value specified with -Z */
      handle_http_prefix:
	http_prefix[0] = '\0';
	mg_printf(conn, "%s", begin);
	mg_printf(conn, "%s", ntop->getPrefs()->get_http_prefix());
	begin = &http_prefix[len];
	http_prefix = strstr(begin, CONST_HTTP_PREFIX_STRING);

	if(http_prefix != NULL)
	  goto handle_http_prefix;
	else
	  mg_printf(conn, "%s", begin);
      }
    }

    fclose(fd);
    return(CONST_LUA_OK);
  } else {
    ntop->getTrace()->traceEvent(TRACE_INFO, "Unable to read file %s", fname);
    return(CONST_LUA_ERROR);
  }
}

/* ****************************************** */

/**
 * @brief Get default interface name.
 * @details Push the default interface name of ntop into the lua stack.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK.
 */
static int ntop_get_default_interface_name(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushstring(vm, ntop->getInterfaceAtId(0)->get_name());
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Set the name of active interface id into lua stack.
 *
 * @param vm The lua stack.
 * @return @ref CONST_LUA_OK.
 */
static int ntop_set_active_interface_id(lua_State* vm) {
  NetworkInterface *iface;
  u_int32_t id;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  id = (u_int32_t)lua_tonumber(vm, 1);

  iface = ntop->getInterfaceById(id);

  // ntop->getTrace()->traceEvent(TRACE_ERROR, "Index: %d, Name: %s", id, iface->get_name());

  if(iface != NULL)
    lua_pushstring(vm, iface->get_name());
  else
    lua_pushnil(vm);

  return(CONST_LUA_OK);
}

/* ****************************************** */
/**
 * @brief Get the ntopng interface names.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK.
 */
static int ntop_get_interface_names(lua_State* vm) {
  lua_newtable(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  for(int i=0; i<ntop->get_num_interfaces(); i++) {
    NetworkInterface *iface =  ntop->getInterfaceAtId(i);

    if(iface != NULL) {
      char num[8];

      snprintf(num, sizeof(num), "%d", i);
      lua_push_str_table_entry(vm, num, iface->get_name());
    }
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static patricia_tree_t* get_allowed_nets(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, CONST_ALLOWED_NETS);
  return((patricia_tree_t*)lua_touserdata(vm, lua_gettop(vm)));
}

/* ****************************************** */

static NetworkInterface* get_ntop_interface(lua_State* vm) {
  NetworkInterface *ntop_interface;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop_interface = handle_null_interface(vm);
  }

  return(ntop_interface);
}

/* ****************************************** */
/**
 * @brief Flush the host contacts of the network interface defined into lua.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK if the network_interface lua variable is not NULL, @ref CONST_LUA_ERROR otherwise.
 */
static int ntop_flush_host_contacts(lua_State* vm) {
  NetworkInterface *ntop_interface;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  ntop_interface = get_ntop_interface(vm);

  if(ntop_interface) {
    ntop_interface->flushHostContacts();
    return(CONST_LUA_OK);
  } else
    return(CONST_LUA_ERROR);
}

/* ****************************************** */

/**
 * @brief Find the network interface and set it as global variable to lua.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK
 */
static int ntop_find_interface(lua_State* vm) {
  char *ifname;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  ifname = (char*)lua_tostring(vm, 1);

  lua_pushlightuserdata(vm, (char*)ntop->getNetworkInterface(ifname));
  lua_setglobal(vm, "ntop_interface");

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the ndpi statistics of interface.
 * @details Get the ntop interface global variable of lua, get nDpistats of interface and push it into lua stack.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK
 */
static int ntop_get_ndpi_interface_stats(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  NdpiStats stats;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_interface) {
    ntop_interface->getnDPIStats(&stats);

    lua_newtable(vm);
    stats.lua(ntop_interface, vm);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the ndpi protocol name of protocol id of network interface.
 * @details Get the ntop interface global variable of lua. Once do that, get the protocol id of lua stack and return into lua stack "Host-to-Host Contact" if protocol id is equal to host family id; the protocol name or null otherwise.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_ndpi_protocol_name(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  NdpiStats stats;
  int proto;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  proto = (u_int32_t)lua_tonumber(vm, 1);

  if(proto == HOST_FAMILY_ID)
    lua_pushstring(vm, "Host-to-Host Contact");
  else {
    if(ntop_interface)
      lua_pushstring(vm, ntop_interface->get_ndpi_proto_name(proto));
    else
      lua_pushnil(vm);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Same as ntop_get_ndpi_protocol_name() with the exception that the protocl breed is returned
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_ndpi_protocol_breed(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  NdpiStats stats;
  int proto;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  proto = (u_int32_t)lua_tonumber(vm, 1);

  if(proto == HOST_FAMILY_ID)
    lua_pushstring(vm, "Unrated-to-Host Contact");
  else {
    if(ntop_interface)
      lua_pushstring(vm, ntop_interface->get_ndpi_proto_breed_name(proto));
    else
      lua_pushnil(vm);
  }

  return(CONST_LUA_OK);
}

/**
 * @brief Get the hosts of network interface.
 * @details Get the ntop interface global variable of lua and return into lua stack a new hash table of host information (Host name and number of bytes sent and received).
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_hosts(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_interface) ntop_interface->getActiveHostsList(vm, get_allowed_nets(vm), false);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the hosts information of network interface.
 * @details Get the ntop interface global variable of lua and return into lua stack a new hash table of hash tables containing the host information.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_hosts_info(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  bool show_details;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  /* Optional */
  if(lua_type(vm, 1) != LUA_TBOOLEAN)
    show_details = true;
  else
    show_details = lua_toboolean(vm, 1) ? true : false;

  if(ntop_interface) ntop_interface->getActiveHostsList(vm, get_allowed_nets(vm), show_details);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the aggregated host information of network interface.
 * @details Get the family, the host name from the lua stack and the ntop interface global variable of lua and return into lua stack a new hash table of hash tables containing the aggregated host information.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_aggregated_hosts_info(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int16_t family = 0;
  char *host = NULL;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(lua_type(vm, 1) == LUA_TNUMBER)
    family = (u_int16_t)lua_tonumber(vm, 1);

  if(lua_type(vm, 2) == LUA_TSTRING)
    host = (char*)lua_tostring(vm, 2);

  if(ntop_interface) ntop_interface->getActiveAggregatedHostsList(vm, get_allowed_nets(vm), family, host);
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the number of aggregated hosts of network interface.
 * @details Get the ntop interface global variable of lua and return into lua stack the number of aggregated hosts.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_num_aggregated_hosts(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_interface) lua_pushnumber(vm, ntop_interface->getNumAggregatedHosts());

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if the specified path is a directory and it exists.
 * @details True if if the specified path is a directory and it exists, false otherwise.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK
 */
static int ntop_is_dir(lua_State* vm) {
  char *path;
  struct stat buf;
  int rc;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  path = (char*)lua_tostring(vm, 1);

  rc = ((stat(path, &buf) != 0) || (!S_ISDIR(buf.st_mode))) ? 0 : 1;
  lua_pushboolean(vm, rc);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if the file or directory exists.
 * @details Get the path of file/directory from to lua stack and push true into lua stack if it exists, false otherwise.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK
 */
static int ntop_get_file_dir_exists(lua_State* vm) {
  char *path;
  struct stat buf;
  int rc;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  path = (char*)lua_tostring(vm, 1);

  rc = (stat(path, &buf) != 0) ? 0 : 1;
  //   ntop->getTrace()->traceEvent(TRACE_ERROR, "%s: %d", path, rc);
  lua_pushboolean(vm, rc);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Return the epoch of the file last change
 * @details This function return that time (epoch) of the last chnge on a file, or -1 if the file does not exist.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK
 */
static int ntop_get_file_last_change(lua_State* vm) {
  char *path;
  struct stat buf;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  path = (char*)lua_tostring(vm, 1);

  if(stat(path, &buf) == 0)
    lua_pushnumber(vm, buf.st_mtime);
  else
    lua_pushnumber(vm, -1); /* not found */

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if ntop has seen VLAN tagged packets on this interface.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_has_vlans(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_interface)
    lua_pushboolean(vm, ntop_interface->hasSeenVlanTaggedPackets());
  else
    lua_pushboolean(vm, 0);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if ntop has loaded ASN information (via GeoIP)
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_has_geoip(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushboolean(vm, ntop->getGeolocation() ? 1 : 0);
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if ntop is running on windows.
 * @details Push into lua stack 1 if ntop is running on windows, 0 otherwise.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_is_windows(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushboolean(vm,
#ifdef WIN32
		  1
#else
		  0
#endif
		  );

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Scan the input directory and return the list of files.
 * @details Get the path from the lua stack and push into a new hashtable the files name existing in the directory.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_list_dir_files(lua_State* vm) {
  char *path;
  DIR *dirp;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  path = (char*)lua_tostring(vm, 1);
  ntop->fixPath(path);

  lua_newtable(vm);

  if((dirp = opendir(path)) != NULL) {
    struct dirent *dp;

    while ((dp = readdir(dirp)) != NULL)
      if(dp->d_name && (dp->d_name[0] != '.')) {
	lua_push_str_table_entry(vm, dp->d_name, dp->d_name);
      }
    (void)closedir(dirp);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the system time and push it into the lua stack.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_gettimemsec(lua_State* vm) {
  struct timeval tp;
  double ret;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  gettimeofday(&tp, NULL);

  ret = (((double)tp.tv_usec) / (double)1000) + tp.tv_sec;

  lua_pushnumber(vm, ret);
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Lua-equivaled ot C inet_ntoa
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_inet_ntoa(lua_State* vm) {
  u_int32_t ip;
  struct in_addr in;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(lua_type(vm, 1) == LUA_TSTRING)
    ip = atol((char*)lua_tostring(vm, 1));
  else if(lua_type(vm, 1) == LUA_TNUMBER)
    ip = (u_int32_t)lua_tonumber(vm, 1);
  else
    return(CONST_LUA_ERROR);

  in.s_addr = htonl(ip);
  lua_pushstring(vm, inet_ntoa(in));
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_zmq_connect(lua_State* vm) {
  char *endpoint, *topic;
  void *context, *subscriber;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((endpoint = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((topic = (char*)lua_tostring(vm, 2)) == NULL)     return(CONST_LUA_PARAM_ERROR);

  context = zmq_ctx_new(), subscriber = zmq_socket(context, ZMQ_SUB);

  if(zmq_connect(subscriber, endpoint) != 0) {
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return(CONST_LUA_PARAM_ERROR);
  }

  if(zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, topic, strlen(topic)) != 0) {
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return -1;
  }

  lua_pushlightuserdata(vm, context);
  lua_setglobal(vm, "zmq_context");

  lua_pushlightuserdata(vm, subscriber);
  lua_setglobal(vm, "zmq_subscriber");

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Delete the specified member(field) from the redis hash stored at key.
 * @details Get the key parameter from the lua stack and delete it from redis.
 *
 * @param vm The lua stack.
 * @return CONST_LUA_OK.
 */
static int ntop_delete_redis_key(lua_State* vm) {
  char *key;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);
  ntop->getRedis()->del(key);
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the members of a redis set.
 * @details Get the set key form the lua stack and push the mambers name into lua stack.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_get_set_members_redis(lua_State* vm) {
  char *key;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);
  ntop->getRedis()->smembers(vm, key);
  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Delete the specified member(field) from the redis hash stored at key.
 * @details Get the member name and the hash key form the lua stack and remove the specified member.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_delete_hash_redis_key(lua_State* vm) {
  char *key, *member;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((member = (char*)lua_tostring(vm, 2)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  ntop->getRedis()->hashDel(key, member);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_zmq_disconnect(lua_State* vm) {
  void *context, *subscriber;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, "zmq_context");
  if((context = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL context");
    return(CONST_LUA_ERROR);
  }

  lua_getglobal(vm, "zmq_subscriber");
  if((subscriber = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL subscriber");
    return(CONST_LUA_ERROR);
  }

  zmq_close(subscriber);
  zmq_ctx_destroy(context);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_zmq_receive(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  void *subscriber;
  int size;
  struct zmq_msg_hdr h;
  char *payload;
  int payload_len;
  zmq_pollitem_t item;
  int rc;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, "zmq_subscriber");
  if((subscriber = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL subscriber");
    return(CONST_LUA_ERROR);
  }

  item.socket = subscriber;
  item.events = ZMQ_POLLIN;
  do {
    rc = zmq_poll(&item, 1, 1000);
    if(rc < 0 || !ntop_interface->isRunning()) return(CONST_LUA_PARAM_ERROR);
  } while (rc == 0);

  size = zmq_recv(subscriber, &h, sizeof(h), 0);

  if(size != sizeof(h) || h.version != MSG_VERSION) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Unsupported publisher version [%d]", h.version);
    return -1;
  }

  payload_len = h.size + 1;
  if((payload = (char*)malloc(payload_len)) != NULL) {
    size = zmq_recv(subscriber, payload, payload_len, 0);
    payload[h.size] = '\0';

    if(size > 0) {
      json_object *o = json_tokener_parse(payload);

      if(o != NULL) {
	struct json_object_iterator it = json_object_iter_begin(o);
	struct json_object_iterator itEnd = json_object_iter_end(o);

	while (!json_object_iter_equal(&it, &itEnd)) {
	  char *key   = (char*)json_object_iter_peek_name(&it);
	  const char *value = json_object_get_string(json_object_iter_peek_value(&it));

	  ntop->getTrace()->traceEvent(TRACE_NORMAL, "[%s]=[%s]", key, value);

	  json_object_iter_next(&it);
	}

	json_object_put(o);
      }

      lua_pushfstring(vm, "%s", payload);
      ntop->getTrace()->traceEvent(TRACE_INFO, "[%u] %s", h.size, payload);
      free(payload);
      return(CONST_LUA_OK);
    } else {
      free(payload);
      return(CONST_LUA_PARAM_ERROR);
    }
  } else
    return(CONST_LUA_PARAM_ERROR);
}

/* ****************************************** */

/**
 * @brief Check if the trace level of ntop is verbose.
 * @details Push true into the lua stack if the trace level of ntop is set to MAX_TRACE_LEVEL, false otherwise.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_verbose_trace(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushboolean(vm, (ntop->getTrace()->get_trace_level() == MAX_TRACE_LEVEL) ? true : false);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static void get_host_vlan_info(char* lua_ip, char** host_ip,
			       u_int16_t* vlan_id,
			       char *buf, u_int buf_len) {
  char *where, *vlan = NULL;

  snprintf(buf, buf_len, "%s", lua_ip);

  if(((*host_ip) = strtok_r(buf, "@", &where)) != NULL)
    vlan = strtok_r(NULL, "@", &where);

  if(vlan)
    (*vlan_id) = (u_int16_t)atoi(vlan);
}

/* ****************************************** */

/**
 * @brief Get the flow information of network interface.
 * @details Get the ntop interface global variable of lua and push the minimal information of flows into the lua stack as a new hashtable.
 *
 * @param vm The lua state.
 * @return CONST_LUA_OK.
 */
static int ntop_get_interface_flows_info(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip = NULL;
  u_int16_t vlan_id = 0;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(lua_type(vm, 1) == LUA_TSTRING) {
    char buf[64];

    get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

    /* Optional VLAN id */
    if(lua_type(vm, 2) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 2);
  }

  if(ntop_interface) ntop_interface->getActiveFlowsList(vm, get_allowed_nets(vm), host_ip, vlan_id);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the host information of network interface.
 * @details Get the ntop interface global variable of lua, the host ip and optional the VLAN id form the lua stack and push a new hash table of hash tables containing the host information into lua stack.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null or the host is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_host_info(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

  /* Optional VLAN id */
  if(lua_type(vm, 2) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 2);

  if((!ntop_interface) || !ntop_interface->getHostInfo(vm, get_allowed_nets(vm), host_ip, vlan_id))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_correalate_host_activity(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

  /* Optional VLAN id */
  if(lua_type(vm, 2) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 2);

  if((!ntop_interface) || !ntop_interface->correlateHostActivity(vm, get_allowed_nets(vm), host_ip, vlan_id))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_similar_host_activity(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

  /* Optional VLAN id */
  if(lua_type(vm, 2) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 2);

  if((!ntop_interface) || !ntop_interface->similarHostActivity(vm, get_allowed_nets(vm), host_ip, vlan_id))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_host_activitymap(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip;
  GenericHost *h;
  bool aggregated;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!ntop_interface)  return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TBOOLEAN)) return(CONST_LUA_ERROR);
  aggregated = lua_toboolean(vm, 2) ? true : false;

  /* Optional VLAN id */
  if(lua_type(vm, 3) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 3);

  if(!aggregated)
    h = ntop_interface->getHost(host_ip, vlan_id);
  else
    h = ntop_interface->getAggregatedHost(host_ip);

  if(h == NULL)
    return(CONST_LUA_ERROR);
  else {
    if((!aggregated) || h->match(get_allowed_nets(vm))) {
      char *json = h->getJsonActivityMap();

      lua_pushfstring(vm, "%s", json);
      free(json);
    }

    return(CONST_LUA_OK);
  }
}

/* ****************************************** */

/**
 * @brief Restore the host of network interface.
 * @details Get the ntop interface global variable of lua and the IP address of host form the lua stack and restore the host into hash host of network interface.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null or if is impossible to restore the host, CONST_LUA_OK otherwise.
 */
static int ntop_restore_interface_host(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_ip;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_ip, &vlan_id, buf, sizeof(buf));

  if((!ntop_interface) || !ntop_interface->restoreHost(host_ip))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Get the aggregated host information of network interface.
 * @details Get the host name from the lua stack and the ntop interface global variable of lua and return into lua stack a new hash table of hash tables containing the aggregated host information.
 *
 * @param vm The lua state.
 * @return CONST_LUA_ERROR if ntop_interface is null, CONST_LUA_OK otherwise.
 */
static int ntop_get_interface_aggregated_host_info(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_name;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_name, &vlan_id, buf, sizeof(buf));

  if((!ntop_interface) || (!ntop_interface->getAggregatedHostInfo(vm, get_allowed_nets(vm), host_name)))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_aggregation_families(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if((!ntop_interface) || (!ntop_interface->getAggregatedFamilies(vm)))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_aggregregations_for_host(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_name;
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &host_name, &vlan_id, buf, sizeof(buf));

  if((!ntop_interface) || (!ntop_interface->getAggregationsForHost(vm, get_allowed_nets(vm), host_name)))
    return(CONST_LUA_ERROR);
  else
    return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_flows_peers(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *host_name;
  u_int16_t vlan_id = 0;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(lua_type(vm, 1) == LUA_TSTRING) {
    char buf[64];

    get_host_vlan_info((char*)lua_tostring(vm, 1), &host_name, &vlan_id, buf, sizeof(buf));
  } else
    host_name = NULL;

  /* Optional VLAN id */
  if(lua_type(vm, 2) == LUA_TNUMBER) vlan_id = (u_int16_t)lua_tonumber(vm, 2);

  if(ntop_interface) ntop_interface->getFlowPeersList(vm, get_allowed_nets(vm), host_name, vlan_id);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_find_flow_by_key(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int32_t key;
  Flow *f;
  patricia_tree_t *ptree = get_allowed_nets(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  key = (u_int32_t)lua_tonumber(vm, 1);

  if(!ntop_interface) return(false);

  f = ntop_interface->findFlowByKey(key, ptree);

  if(f == NULL)
    return(false);
  else {
    f->lua(vm, ptree, true);
    return(true);
  }
}

/* ****************************************** */

static int ntop_get_interface_find_user_flows(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *key;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  key = (char*)lua_tostring(vm, 1);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  ntop_interface->findUserFlows(vm, key);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_find_pid_flows(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int32_t pid;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  pid = (u_int32_t)lua_tonumber(vm, 1);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  ntop_interface->findPidFlows(vm, pid);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_find_father_pid_flows(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int32_t father_pid;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  father_pid = (u_int32_t)lua_tonumber(vm, 1);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  ntop_interface->findFatherPidFlows(vm, father_pid);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_find_proc_name_flows(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *proc_name;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  proc_name = (char*)lua_tostring(vm, 1);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  ntop_interface->findProcNameFlows(vm, proc_name);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_find_host(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  char *key;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  key = (char*)lua_tostring(vm, 1);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  ntop_interface->findHostsByName(vm, get_allowed_nets(vm), key);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_interface_endpoint(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int8_t id;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(lua_type(vm, 1) != LUA_TNUMBER) /* Optional */
    id = 0;
  else
    id = (u_int8_t)lua_tonumber(vm, 1);

  if(ntop_interface) {
    char *endpoint = ntop_interface->getEndpoint(id);

    lua_pushfstring(vm, "%s", endpoint ? endpoint : "");
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_interface_is_running(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!ntop_interface) return(CONST_LUA_ERROR);
  return(ntop_interface->isRunning());
}

/* ****************************************** */

static int ntop_interface_is_idle(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  if(!ntop_interface) return(CONST_LUA_ERROR);
  return(ntop_interface->idle());
}

/* ****************************************** */

static int ntop_interface_set_idle(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  bool state;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!ntop_interface) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TBOOLEAN)) return(CONST_LUA_ERROR);
  state = lua_toboolean(vm, 1) ? true : false;

  ntop_interface->setIdleState(state);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_interface_name2id(lua_State* vm) {
  char *if_name;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if_name = (char*)lua_tostring(vm, 1);

  lua_pushinteger(vm, ntop->getInterfaceIdByName(if_name));

  return(CONST_LUA_OK);
}

/* ****************************************** */

/*
   Code partially taken from third-party/rrdtool-1.4.7/bindings/lua/rrdlua.c
   and made reentrant
*/

static void reset_rrd_state(void) {
  optind = 0;
  opterr = 0;
  rrd_clear_error();
}

/* ****************************************** */

static const char **make_argv(lua_State * vm, u_int offset) {
  const char **argv;
  int i;
  int argc = lua_gettop(vm) - offset;

  if(!(argv = (const char**)calloc(argc, sizeof (char *))))
    /* raise an error and never return */
    luaL_error(vm, "Can't allocate memory for arguments array");

  /* fprintf(stderr, "%s\n", argv[0]); */
  for(i=0; i<argc; i++) {
    u_int idx = i + offset;
    /* accepts string or number */
    if(lua_isstring(vm, idx) || lua_isnumber(vm, idx)) {
      if(!(argv[i] = (char*)lua_tostring (vm, idx))) {
        /* raise an error and never return */
        luaL_error(vm, "Error duplicating string area for arg #%d", i);
      }
    } else {
      /* raise an error and never return */
      luaL_error(vm, "Invalid arg #%d: args must be strings or numbers", i);
    }

    // ntop->getTrace()->traceEvent(TRACE_NORMAL, "[%d] %s", i, argv[i]);
  }

  return(argv);
}

/* ****************************************** */

static int ntop_rrd_create(lua_State* vm) {
  const char *filename;
  unsigned long pdp_step;
  const char **argv;
  int argc, status, offset = 3;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((filename = (const char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  pdp_step = (unsigned long)lua_tonumber(vm, 2);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s(%s)", __FUNCTION__, filename);

  argc = lua_gettop(vm) - offset;
  argv = make_argv(vm, offset);

  reset_rrd_state();
  status = rrd_create_r(filename, pdp_step, time(NULL), argc, argv);
  free(argv);

  if(status != 0) {
    char *err = rrd_get_error();

    if(err != NULL) {
      luaL_error(vm, err);
      return(CONST_LUA_ERROR);
    }
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_rrd_update(lua_State* vm) {
  const char *filename, *update_arg;
  int status;
  const char *argv[1];

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((filename = (const char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((update_arg = (const char*)lua_tostring(vm, 2)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s(%s) %s", __FUNCTION__, filename, update_arg);

  argv[0] = update_arg;
  reset_rrd_state();
  status = rrd_update_r(filename, NULL, 1, argv);

  if(status != 0) {
    char *err = rrd_get_error();

    if(err != NULL) {
      luaL_error(vm, err);
      return(CONST_LUA_ERROR);
    }
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_rrd_fetch(lua_State* vm) {
  unsigned long i, j, step = 0, ds_cnt = 0;
  rrd_value_t *data, *p;
  char **names, *err;
  const char *filename, *cf, *start_s, *end_s;
  time_t  t, start, end;
  rrd_time_value_t start_tv, end_tv;
  int status;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((filename = (const char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s(%s)", __FUNCTION__, filename);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((cf = (const char*)lua_tostring(vm, 2)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if((lua_type(vm, 3) == LUA_TNUMBER) && (lua_type(vm, 4) == LUA_TNUMBER))
    start = (time_t)lua_tonumber(vm, 3), end = (time_t)lua_tonumber(vm, 4);
  else {
    if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
    if((start_s = (const char*)lua_tostring(vm, 3)) == NULL)  return(CONST_LUA_PARAM_ERROR);

    if((err = rrd_parsetime(start_s, &start_tv)) != NULL) {
      luaL_error(vm, err);
      return(CONST_LUA_PARAM_ERROR);
    }

    if(ntop_lua_check(vm, __FUNCTION__, 4, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
    if((end_s = (const char*)lua_tostring(vm, 4)) == NULL)  return(CONST_LUA_PARAM_ERROR);

    if((err = rrd_parsetime(end_s, &end_tv)) != NULL) {
      luaL_error(vm, err);
      return(CONST_LUA_PARAM_ERROR);
    }

    if(rrd_proc_start_end(&start_tv, &end_tv, &start, &end) == -1)
      return(CONST_LUA_PARAM_ERROR);
  }

  reset_rrd_state();

  status = rrd_fetch_r(filename, cf, &start, &end, &step, &ds_cnt, &names, &data);
  if(status != 0) {
    err = rrd_get_error();

    if(err != NULL) {
      luaL_error(vm, err);
      return(CONST_LUA_ERROR);
    }
  }

  lua_pushnumber(vm, (lua_Number) start);
  lua_pushnumber(vm, (lua_Number) step);
  /* fprintf(stderr, "%lu, %lu, %lu, %lu\n", start, end, step, num_points); */

  /* create the ds names array */
  lua_newtable(vm);
  for(i=0; i<ds_cnt; i++) {
    lua_pushstring(vm, names[i]);
    lua_rawseti(vm, -2, i+1);
    rrd_freemem(names[i]);
  }
  rrd_freemem(names);

  /* create the data points array */
  lua_newtable(vm);
  p = data;
  for(t=start+1, i=0; t<end; t+=step, i++) {
    lua_newtable(vm);
    for(j=0; j<ds_cnt; j++) {
      rrd_value_t value = *p++;

      if(value != DNAN /* Skip NaN */) {
	lua_pushnumber(vm, (lua_Number)value);
	lua_rawseti(vm, -2, j+1);
	// ntop->getTrace()->traceEvent(TRACE_NORMAL, "%u / %.f", t, value);
      }
    }
    lua_rawseti(vm, -2, i+1);
  }
  rrd_freemem(data);

  /* return the end as the last value */
  lua_pushnumber(vm, (lua_Number) end);

  return(5);
}

/* ****************************************** */

static int ntop_http_redirect(lua_State* vm) {
  char *url, str[512];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((url = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  snprintf(str, sizeof(str), "HTTP/1.1 302 Found\r\n"
	   "Location: %s\r\n\r\n"
	   "<html>\n"
	   "<head>\n"
	   "<title>Moved</title>\n"
	   "</head>\n"
	   "<body>\n"
	   "<h1>Moved</h1>\n"
	   "<p>This page has moved to <a href=\"%s\">%s</a>.</p>\n"
	   "</body>\n"
	   "</html>\n", url, url, url);

  lua_pushstring(vm, str);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_http_get(lua_State* vm) {
  char *page, *host, rsp[4096], *out;
  u_int port = 80;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((host = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((page = (char*)lua_tostring(vm, 2)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  out = http_get(host, port, page, rsp, sizeof(rsp));

  lua_pushstring(vm, out ? out : "");

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_http_get_prefix(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushstring(vm, ntop->getPrefs()->get_http_prefix());
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_prefs(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  ntop->getPrefs()->lua(vm);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_load_prefs_defaults(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  ntop->getPrefs()->loadIdleDefaults();
  ntop->getPrefs()->lua(vm);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_increase_drops(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);
  u_int32_t num;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  num = (u_int32_t)lua_tonumber(vm, 1);

  if(ntop_interface)
    ntop_interface->incrDrops(num);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_users(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  ntop->getUsers(vm);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_user_group(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  ntop->getUserGroup(vm);
  return(CONST_LUA_OK);
}


/* ****************************************** */

static int ntop_get_allowed_networks(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  ntop->getAllowedNetworks(vm);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_reset_user_password(lua_State* vm) {
  char *who, *username, *old_password, *new_password;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  /* Username who requested the password change */
  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((who = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 2)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((old_password = (char*)lua_tostring(vm, 3)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 4, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((new_password = (char*)lua_tostring(vm, 4)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if((!Utils::isUserAdministrator(vm)) && (strcmp(who, username)))
    return(CONST_LUA_ERROR);

  return(ntop->resetUserPassword(username, old_password, new_password));
}

/* ****************************************** */

static int ntop_change_user_role(lua_State* vm) {
  char *username, *user_role;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((user_role = (char*)lua_tostring(vm, 2)) == NULL) return(CONST_LUA_PARAM_ERROR);

  return ntop->changeUserRole(username, user_role);
}

/* ****************************************** */

static int ntop_change_allowed_nets(lua_State* vm) {
  char *username, *allowed_nets;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((allowed_nets = (char*)lua_tostring(vm, 2)) == NULL) return(CONST_LUA_PARAM_ERROR);

  return ntop->changeAllowedNets(username, allowed_nets);
}

/* ****************************************** */

static int ntop_post_http_json_data(lua_State* vm) {
  char *username, *password, *url, *json;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);
  
  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((password = (char*)lua_tostring(vm, 2)) == NULL) return(CONST_LUA_PARAM_ERROR);
  
  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((url = (char*)lua_tostring(vm, 3)) == NULL) return(CONST_LUA_PARAM_ERROR);
  
  if(ntop_lua_check(vm, __FUNCTION__, 4, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((json = (char*)lua_tostring(vm, 4)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(Utils::postHTTPJsonData(username, password, url, json))
    return(CONST_LUA_OK);
  else
    return(CONST_LUA_ERROR);
}

/* ****************************************** */

static int ntop_add_user(lua_State* vm) {
  char *username, *full_name, *password, *host_role, *allowed_networks;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((full_name = (char*)lua_tostring(vm, 2)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((password = (char*)lua_tostring(vm, 3)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 4, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((host_role = (char*)lua_tostring(vm, 4)) == NULL) return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 5, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((allowed_networks = (char*)lua_tostring(vm, 5)) == NULL) return(CONST_LUA_PARAM_ERROR);

  return ntop->addUser(username, full_name, password, host_role, allowed_networks);
}

/* ****************************************** */

static int ntop_delete_user(lua_State* vm) {
  char *username;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((username = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  return ntop->deleteUser(username);
}

/* ****************************************** */

static int ntop_resolve_address(lua_State* vm) {
  char *numIP, symIP[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_PARAM_ERROR);
  if((numIP = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  ntop->resolveHostName(numIP, symIP, sizeof(symIP));
  lua_pushstring(vm, symIP);
  return(CONST_LUA_OK);
}

/* ****************************************** */

void lua_push_str_table_entry(lua_State *L, const char *key, char *value) {
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_nil_table_entry(lua_State *L, const char *key) {
  lua_pushstring(L, key);
  lua_pushnil(L);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_bool_table_entry(lua_State *L, const char *key, bool value) {
  lua_pushstring(L, key);
  lua_pushboolean(L, value ? 1 : 0);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_int_table_entry(lua_State *L, const char *key, u_int64_t value) {
  lua_pushstring(L, key);
  /* using LUA_NUMBER (double: 64 bit) in place of LUA_INTEGER (ptrdiff_t: 32 or 64 bit
   * according to the platform, as defined in luaconf.h) to handle big counters */
  lua_pushnumber(L, (lua_Number)value);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_int32_table_entry(lua_State *L, const char *key, int32_t value) {
  lua_pushstring(L, key);
  lua_pushnumber(L, (lua_Number)value);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_float_table_entry(lua_State *L, const char *key, float value) {
  lua_pushstring(L, key);
  lua_pushnumber(L, value);
  lua_settable(L, -3);
}

/* ****************************************** */

static int ntop_get_interface_stats(lua_State* vm) {
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_interface) ntop_interface->lua(vm);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_is_pro(lua_State *vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushboolean(vm, 
#ifdef NTOPNG_PRO
		  1
#else
		  0
#endif
		  );

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_dirs(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_newtable(vm);
  lua_push_str_table_entry(vm, "installdir", ntop->get_install_dir());
  lua_push_str_table_entry(vm, "workingdir", ntop->get_working_dir());
  lua_push_str_table_entry(vm, "scriptdir", ntop->getPrefs()->get_scripts_dir());
  lua_push_str_table_entry(vm, "httpdocsdir", ntop->getPrefs()->get_docs_dir());
  lua_push_str_table_entry(vm, "callbacksdir", ntop->getPrefs()->get_callbacks_dir());

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_uptime(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_pushinteger(vm, ntop->getGlobals()->getUptime());
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_info(lua_State* vm) {
  char rsp[256];
  int major, minor, patch;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_newtable(vm);
  lua_push_str_table_entry(vm, "copyright", (char*)"&copy; 1998-2014 - ntop.org");
  lua_push_str_table_entry(vm, "authors", (char*)"The ntop.org team");
  lua_push_str_table_entry(vm, "license", (char*)"GNU GPLv3");
  snprintf(rsp, sizeof(rsp), "%s (%s)",
	   PACKAGE_VERSION, NTOPNG_SVN_RELEASE);
  lua_push_str_table_entry(vm, "version", rsp);
  snprintf(rsp, sizeof(rsp), "%s (%s)", PACKAGE_OSNAME, PACKAGE_MACHINE);
  lua_push_str_table_entry(vm, "platform", rsp);
  lua_push_int_table_entry(vm, "uptime", ntop->getGlobals()->getUptime());
  lua_push_str_table_entry(vm, "version.rrd", rrd_strversion());
  lua_push_str_table_entry(vm, "version.redis", ntop->getRedis()->getVersion(rsp, sizeof(rsp)));
  lua_push_str_table_entry(vm, "version.httpd", (char*)mg_version());
  lua_push_str_table_entry(vm, "version.luajit", (char*)LUAJIT_VERSION);
#ifdef HAVE_GEOIP
  lua_push_str_table_entry(vm, "version.geoip", (char*)GeoIP_lib_version());
#endif
  lua_push_str_table_entry(vm, "version.ndpi", ndpi_revision());

#ifdef NTOPNG_PRO
  lua_push_str_table_entry(vm, "pro.release", NTOPNG_PRO_SVN_RELEASE);
#endif

  zmq_version(&major, &minor, &patch);
  snprintf(rsp, sizeof(rsp), "%d.%d.%d", major, minor, patch);
  lua_push_str_table_entry(vm, "version.zmq", rsp);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_resolved_address(lua_State* vm) {
  char *key, *tmp,rsp[256],value[64];
  Redis *redis = ntop->getRedis();
  u_int16_t vlan_id = 0;
  char buf[64];

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  get_host_vlan_info((char*)lua_tostring(vm, 1), &key, &vlan_id, buf, sizeof(buf));

  if(key == NULL)
    return(CONST_LUA_ERROR);

  if(redis->getAddress(key, rsp, sizeof(rsp), true) == 0)
    tmp = rsp;
  else
    tmp = key;

  if(vlan_id != 0)
    snprintf(value, sizeof(value), "%s@%u", tmp, vlan_id);
  else
    snprintf(value, sizeof(value), "%s", tmp);

#if 0
  if(!strcmp(value, key)) {
    char rsp[64];

    if((ntop->getRedis()->hashGet((char*)HOST_LABEL_NAMES, key, rsp, sizeof(rsp)) == 0)
       && (rsp[0] !='\0'))
      lua_pushfstring(vm, "%s", rsp);
    else
      lua_pushfstring(vm, "%s", value);
  } else
    lua_pushfstring(vm, "%s", value);
#else
  lua_pushfstring(vm, "%s", value);
#endif

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_snmp_get_fctn(lua_State* vm, int operation) {
  char *agent_host, *oid, *community;
  u_int agent_port = 161, timeout = 5, request_id = time(NULL);
  int sock, i = 0, rc = CONST_LUA_OK;
  SNMPMessage *message;
  int len;
  unsigned char *buf;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING))  return(CONST_LUA_ERROR);
  agent_host = (char*)lua_tostring(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING))  return(CONST_LUA_ERROR);
  community = (char*)lua_tostring(vm, 2);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING))  return(CONST_LUA_ERROR);
  oid = (char*)lua_tostring(vm, 3);

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if(sock < 0) return(CONST_LUA_ERROR);

  message = snmp_create_message();
  snmp_set_version(message, 0);
  snmp_set_community(message, community);
  snmp_set_pdu_type(message, operation);
  snmp_set_request_id(message, request_id);
  snmp_set_error(message, 0);
  snmp_set_error_index(message, 0);
  snmp_add_varbind_null(message, oid);

  /* Add additional OIDs */
  i = 4; 
  while(lua_type(vm, i) == LUA_TSTRING) {
    snmp_add_varbind_null(message, (char*)lua_tostring(vm, i));
    i++;
  }

  len = snmp_message_length(message);
  buf = (unsigned char*)malloc(len);
  snmp_render_message(message, buf);
  snmp_destroy_message(message);

  send_udp_datagram(buf, len, sock, agent_host, agent_port);

  free(buf);

  if(input_timeout(sock, timeout) == 0) {
    /* Timeout */
    rc = CONST_LUA_ERROR;
    lua_pushnil(vm);
  } else {
    char buf[BUFLEN];
    SNMPMessage *message;
    char *sender_host, *oid_str,  *value_str;
    int sender_port, added = 0, len;

    len = receive_udp_datagram(buf, BUFLEN, sock, &sender_host, &sender_port);
    message = snmp_parse_message(buf, len);

    i = 0;
    while(snmp_get_varbind_as_string(message, i, &oid_str, NULL, &value_str)) {
      if(!added) lua_newtable(vm), added = 1;
      lua_push_str_table_entry(vm, oid_str, value_str);
      i++;
    }

    snmp_destroy_message(message);

    if(!added) 
      lua_pushnil(vm), rc = CONST_LUA_ERROR;
  }
  close(sock);

  return(rc);
}
/* ****************************************** */

static int ntop_snmpget(lua_State* vm)     { return(ntop_snmp_get_fctn(vm, SNMP_GET_REQUEST_TYPE)); }
static int ntop_snmpgetnext(lua_State* vm) { return(ntop_snmp_get_fctn(vm, SNMP_GETNEXT_REQUEST_TYPE)); }

/* ****************************************** */

/**
 * @brief Send a message to the system syslog
 * @details Send a message to the syslog syslog: callers can specify if it is an error or informational message
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_ERROR if the expected type is equal to function type, @ref CONST_LUA_PARAM_ERROR otherwise.
 */
static int ntop_syslog(lua_State* vm) {
#ifndef WIN32
  char *msg;
  bool is_error;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TBOOLEAN)) return(CONST_LUA_ERROR);
  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING))  return(CONST_LUA_ERROR);

  is_error = lua_toboolean(vm, 1) ? true : false;
  msg = (char*)lua_tostring(vm, 2);

  syslog(is_error ? LOG_ERR : LOG_INFO, "%s", msg);
#endif

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Generate a random value to prevent CSRF and XSRF attacks
 * @details See http://blog.codinghorror.com/preventing-csrf-and-xsrf-attacks/
 *
 * @param vm The lua state.
 * @return The random value just generated
 */
static int ntop_generate_csrf_value(lua_State* vm) {
  char random_a[32], random_b[32], csrf[33];
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  snprintf(random_a, sizeof(random_a), "%d", rand());
  snprintf(random_b, sizeof(random_b), "%lu", time(NULL)*rand());

  mg_md5(csrf, random_a, random_b, NULL);

  redis->set(csrf, (char*)"1", MAX_CSRF_DURATION);
  lua_pushfstring(vm, "%s", csrf);
  return(CONST_LUA_OK);
}

/* ****************************************** */

struct ntopng_sqlite_state {
  lua_State* vm;
  u_int num_rows;
};

static int sqlite_callback(void *data, int argc,
			   char **argv, char **azColName) {
  struct ntopng_sqlite_state *s = (struct ntopng_sqlite_state*)data;

  lua_newtable(s->vm);

  for(int i=0; i<argc; i++)
    lua_push_str_table_entry(s->vm, (const char*)azColName[i],
			     (char*)(argv[i] ? argv[i] : "NULL"));

  lua_pushinteger(s->vm, ++s->num_rows);
  lua_insert(s->vm, -2);
  lua_settable(s->vm, -3);

  return(0);
}

/**
 * @brief Exec SQL query
 * @details Execute the specified query and return the results
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_ERROR in case of error, CONST_LUA_OK otherwise.
 */
static int ntop_sqlite_exec_query(lua_State* vm) {
  char *db_path, *db_query;
  sqlite3 *db;
  char *zErrMsg = 0;
  struct ntopng_sqlite_state state;
  struct stat buf;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING))  return(CONST_LUA_ERROR);
  db_path = (char*)lua_tostring(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING))  return(CONST_LUA_ERROR);
  db_query = (char*)lua_tostring(vm, 2);

  if(stat(db_path, &buf) != 0) {
    ntop->getTrace()->traceEvent(TRACE_INFO, "Not found database %s",
				 db_path);
    return(CONST_LUA_ERROR);
  }

  if(sqlite3_open(db_path, &db)) {
    ntop->getTrace()->traceEvent(TRACE_INFO, "Unable to open %s: %s",
				 db_path, sqlite3_errmsg(db));
    return(CONST_LUA_ERROR);
  }

  state.vm = vm, state.num_rows = 0;
  lua_newtable(vm);
  if(sqlite3_exec(db, db_query, sqlite_callback, (void*)&state, &zErrMsg)) {
    ntop->getTrace()->traceEvent(TRACE_INFO, "SQL Error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);
  return(CONST_LUA_OK);
}

/**
 * @brief Insert a new sampling in the historical database
 * @details Given a certain sampling point, store statistics for said
 *          sampling point.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_PARAM_ERROR in case of wrong parameter,
 *              CONST_LUA_ERROR in case of generic error, CONST_LUA_OK otherwise.
 */
static int ntop_stats_insert_new_sampling(lua_State *vm) {
  char *sampling;
  time_t rawtime;
  tm *timeinfo;
  int ifid;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  ifid = lua_tointeger(vm, 1);
  if(ifid < 0)
    return(CONST_LUA_ERROR);
  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((sampling = (char*)lua_tostring(vm, 2)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  time(&rawtime);
  rawtime -= (rawtime % 60);
  timeinfo = localtime(&rawtime);

  if (ntop->getInterfaceById(ifid)->getStatsManager()->insertSampling(timeinfo, sampling))
    return(CONST_LUA_ERROR);

  return(CONST_LUA_OK);
}

/**
 * @brief Get a sampling from the historical database
 * @details Given a certain sampling point, get statistics for said
 *          sampling point.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_PARAM_ERROR in case of wrong parameter,
 *              CONST_LUA_ERROR in case of generic error, CONST_LUA_OK otherwise.
 */
static int ntop_stats_get_sampling(lua_State *vm) {
  time_t epoch;
  string sampling;
  int ifid;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  ifid = lua_tointeger(vm, 1);
  if(ifid < 0)
    return(CONST_LUA_ERROR);
  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  epoch = (time_t)lua_tointeger(vm, 2);
  epoch -= (epoch % 60);

  if(ntop->getInterfaceById(ifid)->getStatsManager()->getSampling(epoch, &sampling))
    return(CONST_LUA_ERROR);

  lua_pushstring(vm, sampling.c_str());

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_mkdir_tree(lua_State* vm) {
  char *dir;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((dir = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);
  if(dir[0] == '\0')                                   return(CONST_LUA_OK); /* Nothing to do */

  return(Utils::mkdir_tree(dir));
}

/* ****************************************** */

static int ntop_get_redis(lua_State* vm) {
  char *key, *rsp;
  u_int rsp_len = 32768;
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);

  if((rsp = (char*)malloc(rsp_len)) != NULL) {
    lua_pushfstring(vm, "%s", (redis->get(key, rsp, rsp_len) == 0) ? rsp : (char*)"");
    free(rsp);
    return(CONST_LUA_OK);
  } else
    return(CONST_LUA_ERROR);
}

/* ****************************************** */

static int ntop_get_hash_redis(lua_State* vm) {
  char *key, *member, rsp[CONST_MAX_LEN_REDIS_VALUE];
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);
  if((member = (char*)lua_tostring(vm, 2)) == NULL)    return(CONST_LUA_PARAM_ERROR);

  lua_pushfstring(vm, "%s", (redis->hashGet(key, member, rsp, sizeof(rsp)) == 0) ? rsp : (char*)"");

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_set_hash_redis(lua_State* vm) {
  char *key, *member, *value;
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);
  if((member = (char*)lua_tostring(vm, 2)) == NULL)    return(CONST_LUA_PARAM_ERROR);
  if((value  = (char*)lua_tostring(vm, 3)) == NULL)    return(CONST_LUA_PARAM_ERROR);

  redis->hashSet(key, member, value);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_del_hash_redis(lua_State* vm) {
  char *key, *member;
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);
  if((member = (char*)lua_tostring(vm, 2)) == NULL)    return(CONST_LUA_PARAM_ERROR);

  redis->hashDel(key, member);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_hash_keys_redis(lua_State* vm) {
  char *key, **vals;
  Redis *redis = ntop->getRedis();
  int rc;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key    = (char*)lua_tostring(vm, 1)) == NULL)    return(CONST_LUA_PARAM_ERROR);

  rc = redis->hashKeys(key, &vals);

  if(rc > 0) {
    lua_newtable(vm);

    for(int i = 0; i < rc; i++) {
      lua_push_str_table_entry(vm, vals[i], (char*)"");
      free(vals[i]);
    }
    free(vals);
  } else
    lua_pushnil(vm);

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_redis_set_pop(lua_State* vm) {
  char *set_name, rsp[CONST_MAX_LEN_REDIS_VALUE];
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((set_name = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);
  lua_pushfstring(vm, "%s", redis->popSet(set_name, rsp, sizeof(rsp)));

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_redis_get_host_id(lua_State* vm) {
  char *host_name;
  Redis *redis = ntop->getRedis();
  char daybuf[32];
  time_t when = time(NULL);
  bool new_key;
  NetworkInterface *ntop_interface = get_ntop_interface(vm);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((host_name = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  strftime(daybuf, sizeof(daybuf), CONST_DB_DAY_FORMAT, localtime(&when));
  lua_pushinteger(vm, redis->host_to_id(ntop_interface, daybuf, host_name, &new_key));

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_redis_get_id_to_host(lua_State* vm) {
  char *host_idx, rsp[CONST_MAX_LEN_REDIS_VALUE];
  Redis *redis = ntop->getRedis();
  char daybuf[32];
  time_t when = time(NULL);

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((host_idx = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);

  strftime(daybuf, sizeof(daybuf), CONST_DB_DAY_FORMAT, localtime(&when));
  lua_pushfstring(vm, "%d", redis->id_to_host(daybuf, host_idx, rsp, sizeof(rsp)));

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_num_queued_alerts(lua_State* vm) {
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  lua_pushinteger(vm, redis->getNumQueuedAlerts());

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_delete_queued_alert(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  ntop->getRedis()->deleteQueuedAlert((u_int32_t)lua_tonumber(vm, 1));
  lua_pushnil(vm); /* Always return a value to Lua */
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_flush_all_queued_alerts(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  ntop->getRedis()->flushAllQueuedAlerts();
  lua_pushnil(vm); /* Always return a value to Lua */
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_queue_alert(lua_State* vm) {
  Redis *redis = ntop->getRedis();
  int alert_level;
  int alert_type;
  char *alert_msg;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  alert_level = (int)lua_tonumber(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  alert_type = (int)lua_tonumber(vm, 2);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TSTRING)) return(CONST_LUA_ERROR);
  alert_msg = (char*)lua_tostring(vm, 3);

  redis->queueAlert((AlertLevel)alert_level, (AlertType)alert_type, alert_msg);
  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_get_queued_alerts(lua_State* vm) {
  Redis *redis = ntop->getRedis();
  u_int32_t num, i = 0, start_idx = 0, n = 0;
  char *alerts[CONST_MAX_NUM_READ_ALERTS] = { NULL };

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  start_idx = (u_int32_t)lua_tonumber(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  num = (u_int32_t)lua_tonumber(vm, 2);

  if(num < 1) num = 1;
  else if(num > CONST_MAX_NUM_READ_ALERTS) num = CONST_MAX_NUM_READ_ALERTS;

  redis->getQueuedAlerts(get_allowed_nets(vm), alerts, start_idx, num);

  lua_newtable(vm);

  while((i < CONST_MAX_NUM_READ_ALERTS) && (alerts[i] != NULL)) {
    // ntop->getTrace()->traceEvent(TRACE_NORMAL, "%u\t%s", start_idx+n, alerts[i]);
    lua_pushstring(vm, alerts[i]);
    lua_rawseti(vm, -2, n);
    free(alerts[i]);
    i++, n++;
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_redis_dump_daily_stats(lua_State* vm) {
  char *day;
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  /* 131206 */
  if((day = (char*)lua_tostring(vm, 1)) == NULL)  return(CONST_LUA_PARAM_ERROR);
  ntop->getTrace()->traceEvent(TRACE_INFO, "Beginning key dump for %s", day);
  redis->dumpDailyStatsKeys(day);
  ntop->getTrace()->traceEvent(TRACE_INFO, "Keys dumped on disk");

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_set_redis(lua_State* vm) {
  char *key, *value;
  Redis *redis = ntop->getRedis();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(CONST_LUA_PARAM_ERROR);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((value = (char*)lua_tostring(vm, 2)) == NULL)     return(CONST_LUA_PARAM_ERROR);

  if(redis->set(key, value) == 0)
    return(CONST_LUA_OK);
  else
    return(CONST_LUA_ERROR);
}

/* ****************************************** */

static int ntop_lua_http_print(lua_State* vm) {
  struct mg_connection *conn;
  int t;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  lua_getglobal(vm, CONST_HTTP_CONN);
  if((conn = (struct mg_connection*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null HTTP connection");
    return(CONST_LUA_OK);
  }

  switch(t = lua_type(vm, 1)) {
  case LUA_TNIL:
    mg_printf(conn, "%s", "nil");
    break;

  case LUA_TBOOLEAN:
    {
      int v = lua_toboolean(vm, 1);

      mg_printf(conn, "%s", v ? "true" : "false");
    }
    break;

  case LUA_TSTRING:
    {
      char *str = (char*)lua_tostring(vm, 1);

      if(str && (strlen(str) > 0))
	mg_printf(conn, "%s", str);
    }
    break;

  case LUA_TNUMBER:
    {
      char str[64];

      snprintf(str, sizeof(str), "%f", (float)lua_tonumber(vm, 1));
      mg_printf(conn, "%s", str);
    }
    break;

  default:
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s(): Lua type %d is not handled",
				 __FUNCTION__, t);
    return(CONST_LUA_ERROR);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

static int ntop_lua_cli_print(lua_State* vm) {
  int t;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  switch(t = lua_type(vm, 1)) {
  case LUA_TSTRING:
    {
      char *str = (char*)lua_tostring(vm, 1);

      if(str && (strlen(str) > 0))
	ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s", str);
    }
    break;

  case LUA_TNUMBER:
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "%f", (float)lua_tonumber(vm, 1));
    break;

  default:
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s(): Lua type %d is not handled",
				 __FUNCTION__, t);
    return(CONST_LUA_ERROR);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Check if the interface is the historical interface
 * @details Push into lua stack 1 if it is, 0 otherwise.
 *
 * @param vm The lua state.
 * @param id Id of the interface.
 *
 * @return CONST_LUA_OK.
 */
static int is_historical_interface(lua_State* vm) {
  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(ntop->getPrefs()->do_dump_flows_on_db()) {
    u_int32_t id, historical_id;

    historical_id = ntop->getHistoricalInterfaceId();

    if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
    id = (u_int32_t)lua_tonumber(vm, 1);

    lua_pushboolean(vm, (historical_id == id) );
  } else
    lua_pushboolean(vm, false );

  return(CONST_LUA_OK);
}

/* ****************************************** */
/**
 * @brief Get Historical Interface configuration
 * @details Push the main information of the interface into the lua stack.
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK
 */
static int get_historical_info(lua_State* vm) {
  HistoricalInterface *iface = NULL;
  iface = (HistoricalInterface*) ntop->getHistoricalInterface();

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);

  if(iface != NULL) {
    lua_newtable(vm);
    lua_push_int_table_entry(vm, "id", iface->get_id());
    lua_push_str_table_entry(vm, "name", iface->get_name());
    lua_push_bool_table_entry(vm, "on_load", iface->is_on_load());
    lua_push_int_table_entry(vm, "interface_id", iface->getDataInterfaceId());
    lua_push_str_table_entry(vm, "interface_name", ntop->get_if_name(iface->getDataInterfaceId()));
    lua_push_int_table_entry(vm, "from_epoch", iface->getFromEpoch());
    lua_push_int_table_entry(vm, "to_epoch", iface->getToEpoch());
    lua_push_int32_table_entry(vm, "num_files", iface->getNumFiles());
    lua_push_int32_table_entry(vm, "open_error", iface->getOpenError());
    lua_push_int32_table_entry(vm, "file_error", iface->getMissingFiles());
    lua_push_int32_table_entry(vm, "query_error", iface->getQueryError());
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */
/**
 * @brief Set Historical Interface configuration
 * @details Require the following parameters:
 *                number from_epoch
 *                number to_epoch
 *                number interface id
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK
 */
static int set_historical_info(lua_State* vm) {
  u_int32_t from_epoch, to_epoch;
  u_int8_t iface_id = 0;
  HistoricalInterface *iface = NULL;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  iface = (HistoricalInterface*) ntop->getHistoricalInterface();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  from_epoch = (u_int32_t)lua_tonumber(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  to_epoch = (u_int32_t)lua_tonumber(vm, 2);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  iface_id = (u_int8_t)lua_tonumber(vm, 3);

  iface->setFromEpoch((time_t) from_epoch);
  iface->setToEpoch((time_t) to_epoch);
  iface->setDataIntrefaceId(iface_id);

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Start loading historical data process based on a time interval
 * @details Require the following parameters:
 *                number from_epoch
 *                number to_epoch
 *                number interface id
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK and push the return code into the Lua stack
 */
static int load_historical_interval(lua_State* vm) {
  u_int32_t from_epoch, to_epoch;
  u_int8_t iface_id = 0;
  HistoricalInterface *iface = NULL;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  iface = (HistoricalInterface*) ntop->getHistoricalInterface();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  from_epoch = (u_int32_t)lua_tonumber(vm, 1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  to_epoch = (u_int32_t)lua_tonumber(vm, 2);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TNUMBER)) return(CONST_LUA_ERROR);
  iface_id = (u_int8_t)lua_tonumber(vm, 3);

  if((iface == NULL) || iface->is_on_load())
    lua_pushboolean(vm,false);
  else {
    iface->startLoadData((time_t) from_epoch, (time_t) to_epoch, iface_id);
    lua_pushboolean(vm,true);
  }

  return(CONST_LUA_OK);
}

/* ****************************************** */

/**
 * @brief Load historical data from a single file
 * @details Require the following parameters:
 *                string absolute file path
 *                boolean it true cleanup the interface, default value is false
 *
 * @param vm The lua state.
 * @return @ref CONST_LUA_OK and push the return code into the Lua stack
 */
static int load_historical_file(lua_State* vm) {
  char *file_name;
  bool cleanup;
  HistoricalInterface *iface = NULL;

  ntop->getTrace()->traceEvent(TRACE_INFO, "%s() called", __FUNCTION__);
  if(!Utils::isUserAdministrator(vm)) return(CONST_LUA_ERROR);

  iface = (HistoricalInterface*)ntop->getHistoricalInterface();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(CONST_LUA_ERROR);
  if((file_name = (char*)lua_tostring(vm, 1)) == NULL) return(CONST_LUA_PARAM_ERROR);

  /* Optional */
  if(lua_type(vm, 2) != LUA_TBOOLEAN)
    cleanup = false;
  else
    cleanup = lua_toboolean(vm, 2) ? true : false;

  if(cleanup) iface->cleanup();
  lua_pushnumber(vm, iface->loadData(file_name));

  return(CONST_LUA_OK);
}

/* ****************************************** */

typedef struct {
  const char *class_name;
  const luaL_Reg *class_methods;
} ntop_class_reg;

static const luaL_Reg ntop_interface_reg[] = {
  { "getDefaultIfName",       ntop_get_default_interface_name },
  { "setActiveInterfaceId",   ntop_set_active_interface_id },
  { "getIfNames",             ntop_get_interface_names },
  { "find",                   ntop_find_interface },
  { "flushHostContacts",      ntop_flush_host_contacts },
  { "getStats",               ntop_get_interface_stats },
  { "getNdpiStats",           ntop_get_ndpi_interface_stats },
  { "getNdpiProtoName",       ntop_get_ndpi_protocol_name },
  { "getNdpiProtoBreed",      ntop_get_ndpi_protocol_breed },
  { "getHosts",               ntop_get_interface_hosts },
  { "getHostsInfo",           ntop_get_interface_hosts_info },
  { "getAggregatedHostsInfo", ntop_get_interface_aggregated_hosts_info },
  { "getAggregationFamilies", ntop_get_interface_aggregation_families },
  { "getNumAggregatedHosts",  ntop_get_interface_num_aggregated_hosts },
  { "getHostInfo",            ntop_get_interface_host_info },
  { "correlateHostActivity",  ntop_correalate_host_activity },
  { "similarHostActivity",    ntop_similar_host_activity },
  { "getHostActivityMap",     ntop_get_interface_host_activitymap },
  { "restoreHost",            ntop_restore_interface_host },
  { "getAggregatedHostInfo",  ntop_get_interface_aggregated_host_info },
  { "getAggregationsForHost", ntop_get_aggregregations_for_host },
  { "getFlowsInfo",           ntop_get_interface_flows_info },
  { "getFlowPeers",           ntop_get_interface_flows_peers },
  { "findFlowByKey",          ntop_get_interface_find_flow_by_key },
  { "findUserFlows",          ntop_get_interface_find_user_flows },
  { "findPidFlows",           ntop_get_interface_find_pid_flows },
  { "findFatherPidFlows",     ntop_get_interface_find_father_pid_flows },
  { "findNameFlows",          ntop_get_interface_find_proc_name_flows },
  { "findHost",               ntop_get_interface_find_host },
  { "getEndpoint",            ntop_get_interface_endpoint },
  { "incrDrops",              ntop_increase_drops },
  { "isRunning",              ntop_interface_is_running },
  { "isIdle",                 ntop_interface_is_idle },
  { "setInterfaceIdleState",  ntop_interface_set_idle },
  { "name2id",                ntop_interface_name2id },
  /* Historical Interface */
  { "getHistorical",      get_historical_info },
  { "setHistorical",      set_historical_info },
  { "loadHistoricalInterval",      load_historical_interval },
  { "loadHistoricalFile",      load_historical_file},
  { "isHistoricalInterface",      is_historical_interface },
  { NULL,                     NULL }
};

static const luaL_Reg ntop_reg[] = {
  { "getDirs",        ntop_get_dirs },
  { "getInfo",        ntop_get_info },
  { "getUptime",      ntop_get_uptime },
  { "dumpFile",       ntop_dump_file },

  /* Redis */
  { "getCache",       ntop_get_redis },
  { "setCache",       ntop_set_redis },
  { "delCache",       ntop_delete_redis_key },
  { "getMembersCache", ntop_get_set_members_redis },
  { "getHashCache",   ntop_get_hash_redis },
  { "setHashCache",   ntop_set_hash_redis },
  { "delHashCache",   ntop_del_hash_redis },
  { "getHashKeysCache", ntop_get_hash_keys_redis },
  { "delHashCache",   ntop_delete_hash_redis_key },
  { "setPopCache",    ntop_get_redis_set_pop },
  { "dumpDailyStats", ntop_redis_dump_daily_stats },
  { "getHostId",      ntop_redis_get_host_id },
  { "getIdToHost",    ntop_redis_get_id_to_host },

  { "isdir",          ntop_is_dir },
  { "mkdir",          ntop_mkdir_tree },
  { "exists",         ntop_get_file_dir_exists },
  { "fileLastChange", ntop_get_file_last_change },
  { "readdir",        ntop_list_dir_files },
  { "zmq_connect",    ntop_zmq_connect },
  { "zmq_disconnect", ntop_zmq_disconnect },
  { "zmq_receive",    ntop_zmq_receive },

  /* Alerts */
  { "getNumQueuedAlerts",   ntop_get_num_queued_alerts },
  { "getQueuedAlerts",      ntop_get_queued_alerts },
  { "deleteQueuedAlert",    ntop_delete_queued_alert },
  { "flushAllQueuedAlerts", ntop_flush_all_queued_alerts },
  { "queueAlert",           ntop_queue_alert },

  /* Pro */
  { "isPro",          ntop_is_pro },

  /* Historical database */
  { "insertNewSampling",    ntop_stats_insert_new_sampling },
  { "getSampling",          ntop_stats_get_sampling },

  /* Time */
  { "gettimemsec",    ntop_gettimemsec },

  /* Trace */
  { "verboseTrace",   ntop_verbose_trace },

  /* IP */
  { "inet_ntoa",      ntop_inet_ntoa },

  /* RRD */
  { "rrd_create",     ntop_rrd_create },
  { "rrd_update",     ntop_rrd_update },
  { "rrd_fetch",      ntop_rrd_fetch  },

  /* Prefs */
  { "getPrefs",          ntop_get_prefs },
  { "loadPrefsDefaults", ntop_load_prefs_defaults },

  /* HTTP */
  { "httpRedirect",   ntop_http_redirect },
  { "httpGet",        ntop_http_get },
  { "getHttpPrefix",  ntop_http_get_prefix },

  /* Admin */
  { "getUsers",           ntop_get_users },
  { "getUserGroup",       ntop_get_user_group },
  { "getAllowedNetworks", ntop_get_allowed_networks },
  { "resetUserPassword",  ntop_reset_user_password },
  { "changeUserRole",     ntop_change_user_role },
  { "changeAllowedNets",  ntop_change_allowed_nets },
  { "addUser",            ntop_add_user },
  { "deleteUser",         ntop_delete_user },

  /* Security */
  { "getRandomCSRFValue",     ntop_generate_csrf_value },

  /* HTTP */
  { "postHTTPJsonData",       ntop_post_http_json_data },

  /* Address Resolution */
  { "resolveAddress",     ntop_resolve_address },
  { "getResolvedAddress", ntop_get_resolved_address },

  /* Logging */
  { "syslog",         ntop_syslog },

  /* SNMP */
  { "snmpget",        ntop_snmpget },
  { "snmpgetnext",    ntop_snmpgetnext },

  /* SQLite */
  { "execQuery",      ntop_sqlite_exec_query },

  /* Runtime */
  { "hasVLANs",       ntop_has_vlans },
  { "hasGeoIP",       ntop_has_geoip },
  { "isWindows",      ntop_is_windows },

  { NULL,          NULL}
};

/* ****************************************** */

void Lua::lua_register_classes(lua_State *L, bool http_mode) {
  static const luaL_Reg _meta[] = { { NULL, NULL } };
  int i;

  ntop_class_reg ntop[] = {
    { "interface", ntop_interface_reg },
    { "ntop",      ntop_reg },
    {NULL,         NULL}
  };

  luaopen_lsqlite3(L);

  for(i=0; ntop[i].class_name != NULL; i++) {
    int lib_id, meta_id;

    /* newclass = {} */
    lua_createtable(L, 0, 0);
    lib_id = lua_gettop(L);

    /* metatable = {} */
    luaL_newmetatable(L, ntop[i].class_name);
    meta_id = lua_gettop(L);
    luaL_register(L, NULL, _meta);

    /* metatable.__index = class_methods */
    lua_newtable(L), luaL_register(L, NULL, ntop[i].class_methods);
    lua_setfield(L, meta_id, "__index");

    /* class.__metatable = metatable */
    lua_setmetatable(L, lib_id);

    /* _G["Foo"] = newclass */
    lua_setglobal(L, ntop[i].class_name);
  }

  if(http_mode) {
    /* Overload the standard Lua print() with ntop_lua_http_print that dumps data on HTTP server */
    lua_register(L, "print", ntop_lua_http_print);
  } else
    lua_register(L, "print", ntop_lua_cli_print);
}

/* ****************************************** */

#if 0
/**
 * Iterator over key-value pairs where the value
 * maybe made available in increments and/or may
 * not be zero-terminated.  Used for processing
 * POST data.
 *
 * @param cls user-specified closure
 * @param kind type of the value
 * @param key 0-terminated key for the value
 * @param filename name of the uploaded file, NULL if not known
 * @param content_type mime-type of the data, NULL if not known
 * @param transfer_encoding encoding of the data, NULL if not known
 * @param data pointer to size bytes of data at the
 *              specified offset
 * @param off offset of data in the overall value
 * @param size number of bytes in data available
 * @return MHD_YES to continue iterating,
 *         MHD_NO to abort the iteration
 */
static int post_iterator(void *cls,
			 enum MHD_ValueKind kind,
			 const char *key,
			 const char *filename,
			 const char *content_type,
			 const char *transfer_encoding,
			 const char *data, uint64_t off, size_t size)
{
  struct Request *request = cls;
  char tmp[1024];
  u_int len = min(size, sizeof(tmp)-1);

  memcpy(tmp, &data[off], len);
  tmp[len] = '\0';

  fprintf(stdout, "[POST] [%s][%s]\n", key, tmp);
  return MHD_YES;
}
#endif

/* ****************************************** */

/*
  Run a Lua script from within ntopng (no HTTP GUI)
*/
int Lua::run_script(char *script_path, char *ifname) {
  int rc = 0;

  try {
    luaL_openlibs(L); /* Load base libraries */
    lua_register_classes(L, false); /* Load custom classes */

    if(ifname != NULL) {
      /* Name of the interface for which we are running this script for */
      lua_pushstring(L, ifname);
      lua_setglobal(L, "ifname");
    }

    if((rc = luaL_dofile(L, script_path)) != 0) {
      const char *err = lua_tostring(L, -1);

      ntop->getTrace()->traceEvent(TRACE_WARNING, "Script failure [%s][%s]", script_path, err);
      rc = -1;
    }
  } catch(...) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Script failure [%s][%s]", script_path, ifname);
    rc = -2;
  }

  return(rc);
}

/* ****************************************** */

/* http://www.geekhideout.com/downloads/urlcode.c */

#if 0
/* Converts an integer value to its hex character*/
static char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* ****************************************** */

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
static char* http_encode(char *str) {
  char *pstr = str, *buf = (char*)malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if(isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if(*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
#endif

/* ****************************************** */

/* Converts a hex character to its integer value */
static char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* ****************************************** */

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
static char* http_decode(char *str) {
  char *pstr = str, *buf = (char*)malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if(*pstr == '%') {
      if(pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if(*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* ****************************************** */

int Lua::handle_script_request(struct mg_connection *conn,
			       const struct mg_request_info *request_info,
			       char *script_path) {
  char buf[64], key[64], val[64], *_cookies, user[64] = { '\0' }, outbuf[FILENAME_MAX];
  patricia_tree_t *ptree = NULL;

  luaL_openlibs(L); /* Load base libraries */
  lua_register_classes(L, true); /* Load custom classes */

  lua_pushlightuserdata(L, (char*)conn);
  lua_setglobal(L, CONST_HTTP_CONN);

  /* Put the GET params into the environment */
  lua_newtable(L);
  if(request_info->query_string != NULL) {
    char *_query_string = strdup(request_info->query_string);

    if(_query_string) {
      char *where, *query_string;
      int len = strlen(_query_string)+1;

      ntop->getTrace()->traceEvent(TRACE_INFO, "[HTTP] %s", _query_string);

      if((query_string = (char*)malloc(len)) != NULL) {
	char *tok;

	Utils::urlDecode(_query_string, query_string, len);

	tok = strtok_r(query_string, "&", &where);

	while(tok != NULL) {
	  /* key=val */
	  char *equal = strchr(tok, '=');

	  if(equal) {
	    char *decoded_buf;

	    equal[0] = '\0';
	    if((decoded_buf = http_decode(&equal[1])) != NULL) {
	      FILE *fd;

	      Utils::purifyHTTPparam(tok, true);
	      Utils::purifyHTTPparam(decoded_buf, false);

	      /* Now make sure that decoded_buf is not a file path */
	      if((decoded_buf[0] == '.')
		 && ((fd = fopen(decoded_buf, "r"))
		     || (fd = fopen(realpath(decoded_buf, outbuf), "r")))) {
		ntop->getTrace()->traceEvent(TRACE_WARNING, "Discarded '%s'='%s' as argument is a valid file path",
					     tok, decoded_buf);

		decoded_buf[0] = '\0';
		fclose(fd);
	      }

	      // ntop->getTrace()->traceEvent(TRACE_WARNING, "'%s'='%s'", tok, decoded_buf);

	      if(strcmp(tok, "csrf") == 0) {
		char rsp[32];
		if(ntop->getRedis()->get(decoded_buf, rsp, sizeof(rsp)) == -1) {
		  ntop->getTrace()->traceEvent(TRACE_WARNING, "Invalid CSRF parameter specified [%s]", decoded_buf);
		  return(send_error(conn, 500 /* Internal server error */, "Internal server error: CSRF attack?", PAGE_ERROR, tok, rsp));
		}
	      }

	      lua_push_str_table_entry(L, tok, decoded_buf);
	      free(decoded_buf);
	    }
	  }

	  tok = strtok_r(NULL, "&", &where);
	} /* while */

	free(query_string);
      }

      free(_query_string);
    }
  }
  lua_setglobal(L, "_GET"); /* Like in php */

  /* _SERVER */
  lua_newtable(L);
  lua_push_str_table_entry(L, "HTTP_REFERER", (char*)mg_get_header(conn, "Referer"));
  lua_push_str_table_entry(L, "HTTP_USER_AGENT", (char*)mg_get_header(conn, "User-Agent"));
  lua_push_str_table_entry(L, "SERVER_NAME", (char*)mg_get_header(conn, "Host"));
  lua_setglobal(L, "_SERVER"); /* Like in php */

  /* Cookies */
  lua_newtable(L);
  if((_cookies = (char*)mg_get_header(conn, "Cookie")) != NULL) {
    char *cookies = strdup(_cookies);
    char *tok, *val, *where;

    // ntop->getTrace()->traceEvent(TRACE_WARNING, "=> '%s'", cookies);
    tok = strtok_r(cookies, "=", &where);
    while(tok != NULL) {
      while(tok[0] == ' ') tok++;

      if((val = strtok_r(NULL, ";", &where)) != NULL) {
	lua_push_str_table_entry(L, tok, val);
	// ntop->getTrace()->traceEvent(TRACE_WARNING, "'%s'='%s'", tok, val);
      } else
	break;

      tok = strtok_r(NULL, "=", &where);
    }

    free(cookies);
  }
  lua_setglobal(L, "_COOKIE"); /* Like in php */

  /* Put the _SESSION params into the environment */
  lua_newtable(L);

  mg_get_cookie(conn, "user", user, sizeof(user));
  lua_push_str_table_entry(L, "user", user);
  mg_get_cookie(conn, "session", buf, sizeof(buf));
  lua_push_str_table_entry(L, "session", buf);

  if(user[0] != '\0') {
    snprintf(key, sizeof(key), "ntopng.prefs.%s.ifname", user);
    if(ntop->getRedis()->get(key, val, sizeof(val)) < 0) {
    set_default_if_name_in_session:
      snprintf(val, sizeof(val), "%s", ntop->getInterfaceAtId(0)->get_name());
      lua_push_str_table_entry(L, "ifname", val);
      ntop->getRedis()->set(key, val, 3600 /* 1h */);
    } else {
      goto set_preferred_interface;
    }
  } else {
    // We need to check if ntopng is running with the option --disable-login
    snprintf(key, sizeof(key), "ntopng.prefs.ifname");
    if(ntop->getRedis()->get(key, val, sizeof(val)) < 0) {
      goto set_preferred_interface;
    }

  set_preferred_interface:
    NetworkInterface *iface;

    if((iface = ntop->getInterface(val)) != NULL) {
      /* The specified interface still exists */
      lua_push_str_table_entry(L, "ifname", iface->get_name());
    } else {
      goto set_default_if_name_in_session;
    }
  }

  lua_setglobal(L, "_SESSION"); /* Like in php */

  if(user[0] != '\0') {
    char val[255];

    lua_pushlightuserdata(L, user);
    lua_setglobal(L, "user");

    snprintf(key, sizeof(key), "ntopng.user.%s.allowed_nets", user);

    if((ntop->getRedis()->get(key, val, sizeof(val)) != -1)
       && (val[0] != '\0')) {
      char *what, *net;

      // ntop->getTrace()->traceEvent(TRACE_WARNING, "%s", val);

      ptree = New_Patricia(128);
      net = strtok_r(val, ",", &what);

      while(net != NULL) {
	// ntop->getTrace()->traceEvent(TRACE_ERROR, "%s", net);
	ptree_add_rule(ptree, net);
	net = strtok_r(NULL, ",", &what);
      }

      lua_pushlightuserdata(L, ptree);
      lua_setglobal(L, CONST_ALLOWED_NETS);
    }
  }

  if(luaL_dofile(L, script_path) != 0) {
    const char *err = lua_tostring(L, -1);

    if(ptree) Destroy_Patricia(ptree, free_ptree_data);

    ntop->getTrace()->traceEvent(TRACE_WARNING, "Script failure [%s][%s]", script_path, err);
    return(send_error(conn, 500 /* Internal server error */, "Internal server error", PAGE_ERROR, script_path, err));
  }

  if(ptree) Destroy_Patricia(ptree, free_ptree_data);
  return(CONST_LUA_OK);
}
