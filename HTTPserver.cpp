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

#include <sys/stat.h>

extern "C" {
#include <microhttpd.h>
#undef _GNU_SOURCE
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

static HTTPserver *httpserver;

#define PAGE_NOT_FOUND "<html><head><title>ntop</title></head><body>Page &quot;%s&quot; was not found</body></html>"
#define PAGE_ERROR     "<html><head><title>ntop</title></head><body>Script &quot;%s&quot; returned an error:<p>\n<pre>%s</pre></body></html>"

/* ****************************************** */

int page_not_found(struct MHD_Connection *connection, const char *url) {
  char rsp[4096];

  snprintf(rsp, sizeof(rsp), PAGE_NOT_FOUND, url);

  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(rsp), (void *)rsp, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);

  ntop->getTrace()->traceEvent(TRACE_WARNING, "[HTTP] Page not found %s", url);
  return(ret);
}

/* ****************************************** */

int page_error(struct MHD_Connection *connection, const char *url, const char *err) {
  char rsp[4096];

  snprintf(rsp, sizeof(rsp), PAGE_ERROR, url, err);

  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(rsp), (void *)rsp, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);

  ntop->getTrace()->traceEvent(TRACE_WARNING, "[HTTP] Script error %s", url);
  return(ret);
}

/* ****************************************** */

static ssize_t file_reader(void *cls, uint64_t pos, char *buf, size_t max) {
  FILE *file = (FILE*)cls;
  (void)fseek(file, pos, SEEK_SET);
  return(fread(buf, 1, max, file));
}

/* ****************************************** */

static void http_server_free_callback(void *cls) {
  fclose((FILE*)cls);
}

/* ****************************************** */

static int handle_http_request(void *cls,
			       struct MHD_Connection *connection,
			       const char *url,
			       const char *method,
			       const char *version,
			       const char *upload_data,
			       size_t *upload_data_size, void **ptr) {
  static int aptr;
  struct MHD_Response *response;
  int ret;
  FILE *file;
  struct stat buf;
  char path[255] = { 0 };

  if(ntop->getGlobals()->isShutdown()) return(MHD_YES);

  if(0 != strcmp(method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */

  if(&aptr != *ptr) {
    /* do never respond on first call */
    *ptr = &aptr;
    return MHD_YES;
  }

  *ptr = NULL;                  /* reset when done */

  /* 1 - check if this is a static file */
  snprintf(path, sizeof(path), "%s%s", httpserver->get_docs_dir(),
	   (strlen(url) == 1) ? "/index.html" : url);

  if((stat(path, &buf) == 0) && (S_ISREG (buf.st_mode)))
    file = fopen(path, "rb");
  else
    file = NULL;

  if(file == NULL) {
    /* 2 - check if this a script file */
    snprintf(path, sizeof(path), "%s%s", httpserver->get_scripts_dir(),
	     (strlen(url) == 1) ? "/index.lua" : url);

    if((stat(path, &buf) == 0) && (S_ISREG (buf.st_mode))) {
      Lua *l = new Lua();
      
      ntop->getTrace()->traceEvent(TRACE_INFO, "[HTTP] %s [%s]", url, path);
      
      if(l == NULL) {
	ntop->getTrace()->traceEvent(TRACE_ERROR, "[HTTP] Unable to start LUA interpreter");
	return(page_error(connection, url, "Unable to start Lua interpreter"));
      } else {
	ret = l->handle_script_request(path, cls, connection, url, method, version, upload_data, upload_data_size, ptr);
	delete l;
      }
    } else {
      ret = page_not_found(connection, url);
    }
  } else {
    ntop->getTrace()->traceEvent(TRACE_INFO, "[HTTP] %s [%s]", url, path);

    response = MHD_create_response_from_callback(buf.st_size, 32 * 1024,     /* 32k page size */
						 &file_reader,
						 file,
						 &http_server_free_callback);
    if(response == NULL) {
      fclose(file);
      return MHD_NO;
    }

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    /* fclose(file) is not necessary as the HTTP library does it automatically */
  }

  return ret;
}

/* ****************************************** */

HTTPserver::HTTPserver(u_int16_t _port, const char *_docs_dir, const char *_scripts_dir) {
  port = _port, docs_dir = strdup(_docs_dir), scripts_dir = strdup(_scripts_dir);

  httpd_v4 = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
			   port, NULL, NULL, &handle_http_request, (void*)PAGE_NOT_FOUND,
			   MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			   MHD_OPTION_END);

  if(httpd_v4 == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to start HTTP server (IPv4) on port %d", port);
    exit(-1);
  }

  /* ***************************** */
  
  httpd_v6 = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_IPv6,
			      port, NULL, NULL, &handle_http_request, (void*)PAGE_NOT_FOUND,
			      MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			      MHD_OPTION_END);

  if(httpd_v6 == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to start HTTP server (IPv6) on port %d", port);
  }

  /* ***************************** */

  httpserver = this;
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "HTTP server listening on port %d [%s][%s]",
				      port, docs_dir, scripts_dir);
};

/* ****************************************** */

HTTPserver::~HTTPserver() {
  if(httpd_v4) MHD_stop_daemon(httpd_v4);
  if(httpd_v6) MHD_stop_daemon(httpd_v6);

  free(docs_dir), free(scripts_dir);
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "HTTP server terminated");
};

/* ****************************************** */


