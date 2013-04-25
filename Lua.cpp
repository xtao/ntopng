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

void lua_push_str_table_entry(lua_State *L, const char *key, char *value) {
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_settable(L, -3);
}

/* ****************************************** */

void lua_push_int_table_entry(lua_State *L, const char *key, u_int32_t value) {
  lua_pushstring(L, key);
  lua_pushinteger(L, value);
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
  { "getFlowsInfo",   ntop_get_interface_flows_info },
  { "getFlowPeers",   ntop_get_interface_flows_peers },

  { NULL,             NULL}
};

static const luaL_Reg ntop_reg[] = {
  { "getInfo",     ntop_get_info },
  { "dumpFile",    ntop_dump_file },
  { "getCache",    ntop_get_redis },
  { "setCache",    ntop_set_redis },
  { "getResolvedAddress",    ntop_get_resolved_address },
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

  tmp_response = MHD_create_response_from_fd(MHD_SIZE_UNKNOWN, tmp_file);

  if(luaL_dofile(L, script_path) == 0) {
    fsync(tmp_file);
    /* Don't call fclose(tnmp_file) as the file is closed automatically by the httpd */
    ret = MHD_queue_response(connection, MHD_HTTP_OK, tmp_response);
  } else
    ret = page_error(connection, url, lua_tostring(L, -1));

  MHD_destroy_response(tmp_response);
  //fclose(tmp_file);
  unlink(tmp_filename);

  return(ret);
}
