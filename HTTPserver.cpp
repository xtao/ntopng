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
#undef _GNU_SOURCE
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifdef WIN32
/*
    NOTE

	http://old.nabble.com/please-include-python26_d.lib-in-the-installer-td22737890.html
*/
#undef _DEBUG
#endif
#include "Python.h"
};

static HTTPserver *httpserver;
static struct MHD_Connection *this_connection;
static FILE *tmp_file = NULL;
struct MHD_Response *tmp_response;

#define PAGE "<html><head><title>ntop</title></head><body>Page &quot;%s&quot; was not found</body></html>"

/* ****************************************** */

static int page_not_found(struct MHD_Connection *connection, const char *url) {
  char rsp[4096];

  snprintf(rsp, sizeof(rsp), PAGE, url);

  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(rsp), (void *)rsp, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);

  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_WARNING, "[HTTP] Page not found %s", url);
  return(ret);
}

/* ****************************************** */

static ssize_t file_reader(void *cls, uint64_t pos, char *buf, size_t max) {
  FILE *file = (FILE*)cls;
  (void)fseek(file, pos, SEEK_SET);
  return(fread(buf, 1, max, file));
}

/* ****************************************** */

static void free_callback(void *cls) {
  fclose((FILE*)cls);
}

/* ****************************************** */

static int handle_script_request(char *script_path,
				 void *cls,
				 struct MHD_Connection *connection,
				 const char *url,
				 const char *method,
				 const char *version,
				 const char *upload_data,
				 size_t *upload_data_size, void **ptr) {
  FILE *fd;
  int ret = 0;
  Mutex *mutex = httpserver->get_mutex();

  mutex->lock(__FILE__, __LINE__);
  this_connection = connection;

  tmp_file = tmpfile();
  tmp_response = MHD_create_response_from_fd(MHD_SIZE_UNKNOWN, fileno(tmp_file));

#ifdef WIN32
  fd = PyFile_FromString(script_path, "r");
#else
  fd = fopen(script_path, "r");
#endif

  if(fd != NULL) {
    char buf[4096];

    snprintf(buf, sizeof(buf),
	     "import os\nos.environ['DOCUMENT_ROOT']='%s'\n"
	     "os.environ['REQUEST_METHOD']='GET'\n"
	     "os.environ['QUERY_STRING']='%s'\n",
	     url, "" /* FIX */);

    /* See http://bugs.python.org/issue1159 */
    PyRun_SimpleString(buf);

    /* Run the actual program */
#ifdef WIN32
    /* http://python-forum.org/pythonforum/viewtopic.php?f=15&t=1554&p=6567 */
    PyRun_SimpleFile(PyFile_AsFile(fd), script_path);
#else
    PyRun_SimpleFile(fd, script_path);
#endif

#ifndef WIN32
    fclose(fd);
#endif

    /* File is closed automatically */
    ret = MHD_queue_response(connection, MHD_HTTP_OK, tmp_response);
    MHD_destroy_response(tmp_response);
    mutex->unlock(__FILE__, __LINE__);
  } else
    ret = page_not_found(connection, url);

  return(ret);
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

  if((stat(path, &buf) == 0) && (S_ISREG (buf.st_mode)))
    file = fopen(path, "rb");
  else
    file = NULL;

  if(file == NULL) {
    /* 2 - check if this a script file */
    snprintf(path, sizeof(path), "%s%s", httpserver->get_scripts_dir(), url);
    if((stat(path, &buf) == 0) && (S_ISREG (buf.st_mode))) {
      ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "[HTTP] %s [%s]", url, path);
      ret = handle_script_request(path, cls, connection, url, method, version, upload_data, upload_data_size, ptr);
    } else {
      ret = page_not_found(connection, url);
    }
  } else {
    ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "[HTTP] %s [%s]", url, path);

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
  init_python();

  mutex = new Mutex();
  httpd = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
			    port, NULL, NULL, &handle_http_request, (void*)PAGE,
			    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			    MHD_OPTION_END);
  httpserver = this;

  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "HTTP server listening on port %d [%s][%s]",
				      port, docs_dir, scripts_dir);
};

/* ****************************************** */

HTTPserver::~HTTPserver() {
  MHD_stop_daemon(httpd);
  term_python();
  delete mutex;

  ntopGlobals->getTrace()->traceEvent(trace_generic, TRACE_NORMAL, "HTTP server terminated");
};

/* ****************************************** */

static PyObject* ntop_sendString(PyObject *self, PyObject *args) {
  char *msg;

  if(!PyArg_ParseTuple(args, "s", &msg))
    return(NULL);
  
  fprintf(tmp_file, "%s", msg);
  fflush(tmp_file);

  return PyString_FromString("");
}

/* ****************************************** */

static PyMethodDef ntop_methods[] = {
  { "sendString", ntop_sendString, METH_VARARGS, "" },
  { NULL, NULL, 0, NULL }
};

/* ****************************************** */

void HTTPserver::init_python() {
  Py_SetProgramName((char*)"ntopng");
  Py_Initialize();
  PyEval_InitThreads();

  Py_InitModule("ntop", ntop_methods);
}

/* ****************************************** */

void HTTPserver::term_python() {
  Py_Finalize();   /* Cleaning up the interpreter */
}


