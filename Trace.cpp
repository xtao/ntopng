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

/* ******************************* */

Trace::Trace(char *log_path) {
  log_file_path = log_path ? strdup(log_path) : NULL;
  traceLevel = 2;
};

/* ******************************* */

Trace::~Trace() {
};

/* ******************************* */

void Trace::traceEvent(int eventTraceLevel,
		       const char* file, const int line, const char * format, ...) {
  va_list va_ap;
  struct tm result;

  if(eventTraceLevel <= traceLevel) {
    char buf[8192], out_buf[8192];
    char theDate[32];
    const char *extra_msg = "";
    time_t theTime = time(NULL);
    char *syslogMsg;

    va_start (va_ap, format);

    /* We have two paths - one if we're logging, one if we aren't
     *   Note that the no-log case is those systems which don't support it (WIN32),
     *                                those without the headers !defined(USE_SYSLOG)
     *                                those where it's parametrically off...
     */

    memset(buf, 0, sizeof(buf));
    strftime(theDate, 32, "%d/%b/%Y %H:%M:%S", localtime_r(&theTime, &result));

    vsnprintf(buf, sizeof(buf)-1, format, va_ap);

    if(eventTraceLevel == 0 /* TRACE_ERROR */)
      extra_msg = "ERROR: ";
    else if(eventTraceLevel == 1 /* TRACE_WARNING */)
      extra_msg = "WARNING: ";

    while(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';

    // trace_mutex.lock();
    snprintf(out_buf, sizeof(out_buf), "%s [%s:%d] %s%s", theDate, file, line, extra_msg, buf);

    if(log_file_path != NULL) {
      FILE *fd = fopen(log_file_path, "a");

      if(fd) {
	fprintf(fd, "%s\n", out_buf);
	fclose(fd);
      } else
	printf("%s\n", out_buf);
    } else
      printf("%s\n", out_buf);

    fflush(stdout);

    // trace_mutex.unlock();

    syslogMsg = &out_buf[strlen(theDate)+1];
    if(eventTraceLevel == 0 /* TRACE_ERROR */)
      syslog(LOG_ERR, "%s", syslogMsg);
    else if(eventTraceLevel == 1 /* TRACE_WARNING */)
      syslog(LOG_WARNING, "%s", syslogMsg);

    va_end(va_ap);
  }
}

