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

#ifndef _GETOPT_H
#define _GETOPT_H
#endif

#ifndef LIB_VERSION
#define LIB_VERSION "1.4.7"
#endif

#include "third-party/rrdtool-1.4.7/bindings/lua/rrdlua.c"

#define MSG_VERSION 0

struct zmq_msg_hdr {
  char url[32];
  u_int32_t version;
  u_int32_t size;
};

/* ******************************* */

Lua::Lua() {
  L = luaL_newstate();
}

/* ******************************* */

Lua::~Lua() {
  lua_close(L);
}

/* ******************************* */

static int ntop_lua_check(lua_State* vm, const char* func,
			  int pos, int expected_type) {
  if(lua_type(vm, pos) != expected_type) {
    ntop->getTrace()->traceEvent(TRACE_ERROR,
				 "%s : expected %s, got %s", func,
				 lua_typename(vm, expected_type),
				 lua_typename(vm, lua_type(vm,pos)));
    return(-1);
  }

  return(0);
}

/* ****************************************** */

static int ntop_dump_file(lua_State* vm) {
  char *fname, tmp[1024];
  FILE *f;
  int tmp_file;

  lua_getglobal(vm, "tmp_file");
  tmp_file = (int)lua_tointeger(vm, lua_gettop(vm));

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  if((fname = (char*)lua_tostring(vm, 1)) == NULL)     return(-1);

  if((f = fopen(fname, "r")) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to open file %s", fname);
    return(1);
  }

  while((fgets(tmp, sizeof(tmp), f)) != NULL)
    write(tmp_file, tmp, strlen(tmp));

  fclose(f);

  return(1);
}

/* ****************************************** */

static int ntop_find_interface(lua_State* vm) {
  char *ifname;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  ifname = (char*)lua_tostring(vm, 1);

  lua_pushlightuserdata(vm, (char*)ntop->get_NetworkInterface(ifname));
  lua_setglobal(vm, "ntop_interface");

  return(1);
}

/* ****************************************** */

static int ntop_get_ndpi_interface_stats(lua_State* vm) {
  NetworkInterface *ntop_interface;
  NdpiStats stats;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->getnDPIStats(&stats);

  lua_newtable(vm);  
  stats.lua(ntop_interface, vm);

  return(1);
}

/* ****************************************** */

static int ntop_get_interface_hosts(lua_State* vm) {
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->getActiveHostsList(vm, false);

  return(1);
}

/* ****************************************** */

static int ntop_get_interface_hosts_info(lua_State* vm) {
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->getActiveHostsList(vm, true);

  return(1);
}

/* ****************************************** */

static int ntop_get_file_dir_exists(lua_State* vm) {
  char *path;
  struct stat buf;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  path = (char*)lua_tostring(vm, 1);

  return((stat(path, &buf) != 0) ? 0 : 1);
}

/* ****************************************** */

static int ntop_list_dir_files(lua_State* vm) {
  char *path;
  DIR *dirp;
  struct dirent *dp;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  path = (char*)lua_tostring(vm, 1);

  lua_newtable(vm);

  if((dirp = opendir(path)) != NULL) {
    while ((dp = readdir(dirp)) != NULL)
      if(dp->d_name && (dp->d_name[0] != '.')) {
	lua_push_str_table_entry(vm, dp->d_name, dp->d_name);
      }
    (void)closedir(dirp);
  }

  return(1);
}

/* ****************************************** */

static int ntop_zmq_connect(lua_State* vm) {
  char *endpoint, *topic;
  void *context, *subscriber;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(-1);
  if((endpoint = (char*)lua_tostring(vm, 1)) == NULL)  return(-1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(-1);
  if((topic = (char*)lua_tostring(vm, 2)) == NULL)     return(-1);

  context = zmq_ctx_new(), subscriber = zmq_socket(context, ZMQ_SUB);

  if(zmq_connect(subscriber, endpoint) != 0) {
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return(-1);
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

  return(1);
}

/* ****************************************** */

static int ntop_zmq_disconnect(lua_State* vm) {
  void *context, *subscriber;

  lua_getglobal(vm, "zmq_context");
  if((context = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL context");
    return(0);
  }

  lua_getglobal(vm, "zmq_subscriber");
  if((subscriber = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL subscriber");
    return(0);
  }

  zmq_close(subscriber);
  zmq_ctx_destroy(context);

  return(1);
}

/* ****************************************** */

static int ntop_zmq_receive(lua_State* vm) {
  NetworkInterface *ntop_interface;
  void *subscriber;
  int size;
  struct zmq_msg_hdr h;
  char *payload;
  int payload_len;
  zmq_pollitem_t item;
  int rc;

  lua_getglobal(vm, "zmq_subscriber");
  if((subscriber = (void*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: NULL subscriber");
    return(0);
  }

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  item.socket = subscriber;
  item.events = ZMQ_POLLIN;
  do {
    rc = zmq_poll(&item, 1, 1000);
    if (rc < 0 || !ntop_interface->isRunning()) return(-1);
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
    lua_pushfstring(vm, "%s", payload);
    ntop->getTrace()->traceEvent(TRACE_INFO, "[%u] %s", h.size, payload);
    free(payload);
    return(1);
  } else
    return(-1);
}

/* ****************************************** */

static int ntop_get_interface_flows_info(lua_State* vm) {
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->getActiveFlowsList(vm);

  return(1);
}

/* ****************************************** */

static int ntop_get_interface_host_info(lua_State* vm) {
  NetworkInterface *ntop_interface;
  char *host_ip;
  u_int16_t vlan_id;
  
  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  host_ip = (char*)lua_tostring(vm, 1);

  /* Optional VLAN id */
  if(lua_type(vm, 2) != LUA_TNUMBER) vlan_id = 0; else vlan_id = (u_int16_t)lua_tonumber(vm, 2);

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  if(!ntop_interface->getHostInfo(vm, host_ip, vlan_id))
    return(0);
  else
    return(1);
}

/* ****************************************** */

static int ntop_get_interface_flows_peers(lua_State* vm) {
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->getFlowPeersList(vm);

  return(1);
}

/* ****************************************** */

static int ntop_get_interface_flow_by_key(lua_State* vm) {
  NetworkInterface *ntop_interface;
  u_int32_t key;
  Flow *f;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TNUMBER)) return(0);
  key = (u_int32_t)lua_tonumber(vm, 1);

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(false);
  } else
    f = ntop_interface->findFlowByKey(key);

  if(f == NULL)
    return(false);
  else {
    f->lua(vm, true);
    return(true);
  }
}

/* ****************************************** */

static int ntop_get_interface_endpoint(lua_State* vm) {
  NetworkInterface *ntop_interface;
  char *endpoint;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(false);
  }

  endpoint = ntop_interface->getEndpoint();

  lua_pushfstring(vm, "%s", endpoint ? endpoint : "");

  return(true);
}

/* ****************************************** */

static int ntop_interface_is_running(lua_State* vm) {
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(false);
  }

  return(ntop_interface->isRunning());
}

/* ****************************************** */

static int ntop_process_flow(lua_State* vm) {
  NetworkInterface *ntop_interface;
  IpAddress src_ip, dst_ip;
  u_int16_t src_port, dst_port;
  u_int16_t vlan_id;
  u_int16_t proto_id;
  u_int8_t l4_proto;
  u_int in_pkts, in_bytes, out_pkts, out_bytes;
  char *str;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(-1);
  if((str = (char*)lua_tostring(vm, 1)) == NULL)  return(-1);
  src_ip.set_from_string(str);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(-1);
  if((str = (char*)lua_tostring(vm, 2)) == NULL)     return(-1);
  dst_ip.set_from_string(str);

  if(ntop_lua_check(vm, __FUNCTION__, 3, LUA_TNUMBER)) return(0);
  src_port = (u_int32_t)lua_tonumber(vm, 3);

  if(ntop_lua_check(vm, __FUNCTION__, 4, LUA_TNUMBER)) return(0);
  dst_port = (u_int32_t)lua_tonumber(vm, 4);

  if(ntop_lua_check(vm, __FUNCTION__, 5, LUA_TNUMBER)) return(0);
  vlan_id = (u_int32_t)lua_tonumber(vm, 5);

  if(ntop_lua_check(vm, __FUNCTION__, 6, LUA_TNUMBER)) return(0);
  proto_id = (u_int32_t)lua_tonumber(vm, 6);

  if(ntop_lua_check(vm, __FUNCTION__, 7, LUA_TNUMBER)) return(0);
  l4_proto = (u_int32_t)lua_tonumber(vm, 7);

  if(ntop_lua_check(vm, __FUNCTION__, 8, LUA_TNUMBER)) return(0);
  in_pkts = (u_int32_t)lua_tonumber(vm, 8);

  if(ntop_lua_check(vm, __FUNCTION__, 9, LUA_TNUMBER)) return(0);
  in_bytes = (u_int32_t)lua_tonumber(vm, 9);

  if(ntop_lua_check(vm, __FUNCTION__, 10, LUA_TNUMBER)) return(0);
  out_pkts = (u_int32_t)lua_tonumber(vm, 10);

  if(ntop_lua_check(vm, __FUNCTION__, 11, LUA_TNUMBER)) return(0);
  out_bytes = (u_int32_t)lua_tonumber(vm, 11);

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(false);
  }

  ntop_interface->flow_processing(&src_ip, &dst_ip, src_port, dst_port, vlan_id, proto_id, l4_proto,
    in_pkts, in_bytes, out_pkts, out_bytes);

  return(true);
}

/* ****************************************** */

void lua_push_str_table_entry(lua_State *L, const char *key, char *value) {
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_bool_table_entry(lua_State *L, const char *key, bool value) {
  lua_pushstring(L, key);
  lua_pushboolean(L, value ? 1 : 0);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_int_table_entry(lua_State *L, const char *key, u_int32_t value) {
  lua_pushstring(L, key);
  lua_pushinteger(L, value);
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
  NetworkInterface *ntop_interface;

  lua_getglobal(vm, "ntop_interface");
  if((ntop_interface = (NetworkInterface*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "INTERNAL ERROR: null interface");
    return(0);
  }

  ntop_interface->lua(vm);

  return(1);
}

/* ****************************************** */

static int ntop_get_info(lua_State* vm) {
  char rsp[256];

  lua_newtable(vm);
  snprintf(rsp, sizeof(rsp), "%s (%s)", PACKAGE_VERSION, PACKAGE_RELEASE);
  lua_push_str_table_entry(vm, "version", rsp);
  lua_push_int_table_entry(vm, "uptime", ntop->getGlobals()->getUptime());

  return(1);
}

/* ****************************************** */

static int ntop_get_resolved_address(lua_State* vm) {
  char *key, *value, rsp[256];
  Redis *redis = ntop->getRedis();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(-1);

  if(redis->getAddress(key, rsp, sizeof(rsp)) == 0) {
    value = rsp;
  } else {
    value = key;
  }

  lua_pushfstring(vm, "%s", value);

  return(1);
}

/* ******************************************* */

static void revertSlashIfWIN32(char *str, int mode) {
#ifdef WIN32
  int i;

  for(i=0; str[i] != '\0'; i++)
    switch(mode) {
    case 0:
      if(str[i] == '/') str[i] = '\\';
      //else if(str[i] == ' ') str[i] = '_';
      break;
    case 1:
      if(str[i] == '\\') str[i] = '/';
      break;
    }
#endif
}

/* ****************************************** */

static int ntop_mkdir_tree(lua_State* vm) {
  char *dir, path[256];
  int permission = 0777, i, rc;

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  if((dir = (char*)lua_tostring(vm, 1)) == NULL)       return(-1);
  if(dir[0] == '\0')                                   return(1); /* Nothing to do */

  snprintf(path, sizeof(path), "%s", dir);
	   
  revertSlashIfWIN32(path, 0);

  /* Start at 1 to skip the root */
  for(i=1; path[i] != '\0'; i++)
    if(path[i] == CONST_PATH_SEP) {
#ifdef WIN32
      /* Do not create devices directory */
      if((i > 1) && (path[i-1] == ':')) continue;
#endif

      path[i] = '\0';
      rc = ntop_mkdir(path, permission);
      path[i] = CONST_PATH_SEP;
    }

  rc = ntop_mkdir(path, permission);

  return(rc == 0 ? 1 : -1);
}

/* ****************************************** */

static int ntop_get_redis(lua_State* vm) {
  char *key, *value, rsp[256];
  Redis *redis = ntop->getRedis();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(-1);

  value = (redis->get(key, rsp, sizeof(rsp)) == 0) ? rsp : (char*)"";
  lua_pushfstring(vm, "%s", value);

  return(1);
}

/* ****************************************** */

static int ntop_get_datadir(lua_State* vm) {
  lua_pushfstring(vm, "%s", ntop->get_data_dir());
  return(1);
}

/* ****************************************** */

static int ntop_set_redis(lua_State* vm) {
  char *key, *value;
  Redis *redis = ntop->getRedis();

  if(ntop_lua_check(vm, __FUNCTION__, 1, LUA_TSTRING)) return(0);
  if((key = (char*)lua_tostring(vm, 1)) == NULL)       return(-1);

  if(ntop_lua_check(vm, __FUNCTION__, 2, LUA_TSTRING)) return(0);
  if((value = (char*)lua_tostring(vm, 2)) == NULL)     return(-1);

  if(redis->set(key, value) == 0)
    return(1);
  else
    return(0);
}

/* ****************************************** */

static int ntop_lua_http_print(lua_State* vm) {
  int tmp_file, t;

  lua_getglobal(vm, "tmp_file");
  tmp_file = (int)lua_tointeger(vm, lua_gettop(vm));

  switch(t = lua_type(vm, 1)) {
  case LUA_TSTRING:
    {
      char *str = (char*)lua_tostring(vm, 1);
      if(str && (strlen(str) > 0))
	write(tmp_file, str, strlen(str));
    }
    break;

  case LUA_TNUMBER:
    {
      char str[64];

      snprintf(str, sizeof(str), "%f", (float)lua_tonumber(vm, 1));
      write(tmp_file, str, strlen(str));
    }
    break;

  default:
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s(): Lua type %d is not handled",
				 __FUNCTION__, t);
    return(0);
  }

  return(1);
}

/* ****************************************** */

static int ntop_lua_cli_print(lua_State* vm) {
  int t;
  
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
    return(0);
  }

  return(1);
}

/* ****************************************** */

typedef struct {
  const char *class_name;
  const luaL_Reg *class_methods;
} ntop_class_reg;

static const luaL_Reg ntop_interface_reg[] = {
  { "find",           ntop_find_interface },
  { "getStats",       ntop_get_interface_stats },
  { "getNdpiStats",   ntop_get_ndpi_interface_stats },
  { "getHosts",       ntop_get_interface_hosts },
  { "getHostsInfo",   ntop_get_interface_hosts_info },
  { "getHostInfo",    ntop_get_interface_host_info },
  { "getFlowsInfo",   ntop_get_interface_flows_info },
  { "getFlowPeers",   ntop_get_interface_flows_peers },
  { "findFlowByKey",  ntop_get_interface_flow_by_key },
  { "getEndpoint",    ntop_get_interface_endpoint },
  { "processFlow",    ntop_process_flow },
  { "isRunning",      ntop_interface_is_running },
  { NULL,             NULL}
};

static const luaL_Reg ntop_reg[] = {
  { "getInfo",     ntop_get_info },
  { "dumpFile",    ntop_dump_file },
  { "getDataDir",  ntop_get_datadir },
  { "getCache",    ntop_get_redis },
  { "setCache",    ntop_set_redis },
  { "getResolvedAddress",    ntop_get_resolved_address },
  { "mkdir",       ntop_mkdir_tree },
  { "exists",      ntop_get_file_dir_exists },
  { "readdir",     ntop_list_dir_files },
  { "zmq_connect",    ntop_zmq_connect },
  { "zmq_disconnect", ntop_zmq_disconnect },
  { "zmq_receive",    ntop_zmq_receive },
  { NULL,          NULL}
};

/* ****************************************** */

void Lua::lua_register_classes(lua_State *L, bool http_mode) {
  int lib_id, meta_id;
  static const luaL_Reg _meta[] = { { NULL, NULL } };
  int i;

  ntop_class_reg ntop[] = {
    { "interface", ntop_interface_reg },
    { "ntop",     ntop_reg },
    {NULL,    NULL}
  };

  for(i=0; ntop[i].class_name != NULL; i++) {
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

    /* metatable.__metatable = _meta */
    //luaL_newlib(L, _meta);
    //lua_setfield(L, meta_id, "__metatable");

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

  /* Register RRD bindings */
  luaopen_rrd(L);
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

int MHD_KeyValueIteratorGet(void *cls, enum MHD_ValueKind kind,
			    const char *key, const char *value) {
  lua_State *L = (lua_State*)cls;

  lua_push_str_table_entry(L, key, (char*)value);
  return(MHD_YES);
}

/* ****************************************** */

int Lua::run_script(char *script_path) {
  luaL_openlibs(L); /* Load base libraries */   
  lua_register_classes(L, false); /* Load custom classes */
  return(luaL_dofile(L, script_path));
}

/* ****************************************** */

static ssize_t file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  int tmp_file = *(int*)cls;

  (void) lseek (tmp_file, pos, SEEK_SET);
  
  return read(tmp_file, buf, max);
}

static void file_free_callback (void *cls)
{
  int f = *(int*)cls;
  close(f);
  free(cls);
}

/* ****************************************** */

int Lua::handle_script_request(char *script_path,
			       void *cls,
			       struct MHD_Connection *connection,
			       const char *url,
			       const char *method,
			       const char *version,
			       const char *upload_data,
			       size_t *upload_data_size, void **ptr) {
  int ret = 0;
  MHD_Response *tmp_response;
  char tmp_path[256];
  char *tmp_filename = ntop->getGlobals()->get_temp_filename(tmp_path, sizeof(tmp_path));
  int tmp_file;

  /* Register the connection in the state */
  if((tmp_filename == NULL)
     || ((tmp_file = open(tmp_filename, O_RDWR|O_CREAT)) < 0)) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "[HTTP] tmpnam(%s) error %d [%d/%s]",
				 tmp_filename ? tmp_filename : "", tmp_file,
				 errno, strerror(errno));
    unlink(tmp_filename);
    return(page_not_found(connection, url));
  }

  luaL_openlibs(L); /* Load base libraries */   
  lua_register_classes(L, true); /* Load custom classes */

  lua_pushinteger(L, tmp_file);
  lua_setglobal(L, "tmp_file");

  if(!strcmp(method, MHD_HTTP_METHOD_POST)) {
#if 0
    request->pp = MHD_create_post_processor (connection, 1024, &post_iterator, request);
#endif
  } else {
    /* Put the GET params into the environment */
    lua_newtable(L);
    MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, MHD_KeyValueIteratorGet, (void*)L);
    lua_setglobal(L, "_GET"); /* Like in php */
  }

  if(luaL_dofile(L, script_path) == 0) {
    off_t where;

    fsync(tmp_file);
    where = lseek(tmp_file, 0, SEEK_CUR); /* Get current position */
    lseek(tmp_file, 0, SEEK_SET);
    int *a = (int*)malloc(sizeof(int));
    *a = tmp_file;

    tmp_response = MHD_create_response_from_callback(where, 2048, &file_reader, (void*)a, file_free_callback);

    /* Don't call fclose(tnmp_file) as the file is closed automatically by the httpd */
    ret = MHD_queue_response(connection, MHD_HTTP_OK, tmp_response);
    MHD_destroy_response(tmp_response);
  } else {
    ret = page_error(connection, url, lua_tostring(L, -1)); 
    close(tmp_file);
  }

  unlink(tmp_filename);

  return(ret);
}
