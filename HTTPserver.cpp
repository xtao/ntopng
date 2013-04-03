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

#include "ntop.h"

#include <sys/stat.h>

extern "C" {
#include <microhttpd.h>
};

static HTTPserver *httpserver;

#define PAGE "<html><head><title>libmicrohttpd demo</title></head><body>Query string for &quot;%s&quot; was &quot;%s&quot;</body></html>"

/* ****************************************** */

static ssize_t file_reader(void *cls, uint64_t pos, char *buf, size_t max) {
  FILE *file = (FILE*)cls;
  (void) fseek(file, pos, SEEK_SET);
  return(fread(buf, 1, max, file));
}

/* ****************************************** */

static void free_callback(void *cls) {
  fclose((FILE*)cls);
}

/* ****************************************** */

static int ahc_echo(void *cls,
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
  char path[255];
  
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

  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "[HTTP] %s [%s]", url, path);
  
  if((0 == stat(path, &buf)) && (S_ISREG (buf.st_mode)))
    file = fopen(path, "rb");
  else
    file = NULL;

  if(file == NULL) {
    response = MHD_create_response_from_buffer (strlen (PAGE),
						(void *) PAGE,
						MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response (response);
  } else {
    response = MHD_create_response_from_callback (buf.st_size, 32 * 1024,     /* 32k page size */
						  &file_reader,
						  file,
						  &free_callback);
    if(response == NULL) {
      fclose (file);
      return MHD_NO;
    }

    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
  }

  return ret;
}

/* ****************************************** */

HTTPserver::HTTPserver(u_int16_t _port, const char *_docs_dir, const char *_scripts_dir) {
  port = _port, docs_dir = strdup(_docs_dir), scripts_dir = strdup(_scripts_dir);

  httpd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
			    port, NULL, NULL, &ahc_echo, (void*)PAGE,
			    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			    MHD_OPTION_END);
  httpserver = this;

  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "HTTP server listening on port %d [%s][%s]",
				      port, docs_dir, scripts_dir);
};

/* ****************************************** */

HTTPserver::~HTTPserver() {  
  MHD_stop_daemon(httpd);
  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "HTTP server terminated");
};

/* ****************************************** */
