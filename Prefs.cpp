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

Prefs::Prefs() {
  enable_dns_resolution = sniff_dns_responses = true;
  categorization_enabled = false, resolve_all_host_ip = false;
  host_max_idle = 60 /* sec */, flow_max_idle = 30 /* sec */;
  max_num_hosts = 32768, max_num_flows = 65536;
}

/* ******************************************* */

Prefs::~Prefs() {
  ;
}

/* ******************************************* */

int Prefs::load(const char *path) {
  char buffer[512], *line, *key, *value, *conf_path = (char*) path;
  FILE *fd;
  int i;

  if(conf_path == NULL) {
    snprintf(buffer, sizeof(buffer), "%s/ntopng.conf", ntop->get_data_dir());
    conf_path = buffer;
  }

  fd = fopen(conf_path, "r");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Config file %s not found (it will be created)", conf_path);
    return(-1);
  }

  while(fd) {

    if(!(line = fgets(buffer, sizeof(buffer), fd)))
      break;

    if(((i = strlen(line)) <= 1) || (line[0] == '#'))
      continue;
    else
      line[i-1] = '\0';

    key = line;
    key = Utils::trim(key);

    value = strrchr(line, '=');
    if(value == NULL) {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "Invalid line '%s'", line);
      continue;
    } else
      value[0] = 0, value = &value[1];
    value = Utils::trim(value);

#if 1
    /* inserting all users info into redis */
    if (strncmp(key, "user.", 5) == 0) {
      if(ntop->getRedis()->set(key, value, 0) < 0)
        ntop->getTrace()->traceEvent(TRACE_WARNING, "Error setting '%s' = '%s'", key, value);
    }
#else
    /* TODO read preferences */
    if(strcasecmp(key, ...) == 0) {
      ...
    } else if (strncasecmp(key, "user", 4) == 0) {
      char *holder, *attr, *username;
      if (strtok_r(key, ".", &holder) == NULL) continue;
      if ((username = strtok_r(NULL, ".", &holder)) == NULL) continue;
      if ((attr = strtok_r(NULL, ".", &holder)) == NULL) { /* value is the password */
        /* password in value */
      } else {
        if (strncasecmp(attr, "full_name", 8) == 0) {
          /* fullname in value */
        } else if (strncasecmp(attr, "group", 5) == 0) {
          /* group in value */
        }
      }
    }
#endif

  }

  fclose(fd);

  return(0);
}

/* ******************************************* */

int Prefs::save(const char *path) {
  char buffer[512], *conf_path = (char*) path;
  char **keys;
  char val[64];
  int rc, i;
  FILE *fd;

  if(conf_path == NULL) {
    snprintf(buffer, sizeof(buffer), "%s/ntopng.conf", ntop->get_data_dir());
    conf_path = buffer;
  }

  fd = fopen(conf_path, "w");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to open file %s [%s]", conf_path, strerror(errno));
    return(-1);
  }

  /* TODO write preferences */

  /* wrinting users */
  if((rc = ntop->getRedis()->keys("user.*", &keys)) > 0) {
    for (i = 0; i < rc; i++) {
      if (keys[i] == NULL) continue; /* safety check */

      if(ntop->getRedis()->get(keys[i], val, sizeof(val)) >= 0) 
        fprintf(fd, "%s=%s\n", keys[i], val); 

      free(keys[i]);
    }

    free(keys);
  }

  fclose(fd);

  return(rc);
}

/* ******************************************* */

