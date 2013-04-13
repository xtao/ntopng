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

/* ******************************************* */

PeriodicActivities::PeriodicActivities() {

}

/* ******************************************* */

PeriodicActivities::~PeriodicActivities() {
  void *res;

  pthread_join(periodicLoop, &res);
}

/* **************************************************** */

static void* activitiesLoop(void* ptr) {
  PeriodicActivities *a = (PeriodicActivities*)ptr;

  a->loop();
  return(NULL);
}

/* ******************************************* */

void PeriodicActivities::startPeriodicLoop() {
  ntop->getTrace()->traceEvent(TRACE_NORMAL, "Started periodic activities loop...");

  pthread_create(&periodicLoop, NULL, activitiesLoop, (void*)this);
}

/* ******************************************* */

void PeriodicActivities::runScript(char *path) {
  struct stat statbuf;

  if(stat(path, &statbuf) == 0) {
    Lua *l = new Lua();

    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Running script %s", path);

    l->run_script(path);
    delete l;
  }
}

/* ******************************************* */

void PeriodicActivities::loop() {
  char minute_script[256], hourly_script[256], daily_script[256], *cd = ntop->get_callbacks_dir();

  snprintf(minute_script, sizeof(minute_script), "%s/%s", cd, MINUTE_SCRIPT_PATH);
  snprintf(hourly_script, sizeof(hourly_script), "%s/%s", cd, HOURLY_SCRIPT_PATH);
  snprintf(daily_script, sizeof(daily_script), "%s/%s", cd, DAILY_SCRIPT_PATH);
  
  while(!ntop->getGlobals()->isShutdown()) {
    u_int now = (u_int)time(NULL);

    if((now % 60) == 0) {
      /* Minute script */

      runScript(minute_script);      
    }

    if((now % 3600) == 0) {
      /* Hourly script */

      runScript(hourly_script);
    }

    if((now % 86400) == 0) {
      /* Daily script */

      runScript(daily_script);
    }

    /* We sleep only one second so in case of shutdown we're responsive */
    sleep(1);
  }
}
