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

Prefs::Prefs(Ntop *_ntop) {
  ntop = _ntop;
  ifName = local_networks = NULL;
  enable_dns_resolution = sniff_dns_responses = true;
  categorization_enabled = false, resolve_all_host_ip = false;
  host_max_idle = 60 /* sec */, flow_max_idle = 30 /* sec */;
  max_num_hosts = 32768, max_num_flows = 65536;
  data_dir = strdup(CONST_DEFAULT_DATA_DIR); 
  docs_dir = strdup(CONST_DEFAULT_DOCS_DIR);
  scripts_dir = strdup(CONST_DEFAULT_SCRIPTS_DIR);
  callbacks_dir = strdup(CONST_DEFAULT_CALLBACKS_DIR);
  users_file_path = strdup(CONST_DEFAULT_USERS_FILE);
  config_file_path = ndpi_proto_path = NULL;
  http_port = CONST_DEFAULT_NTOP_PORT;
  change_user = true;
  localnets = false;
  categorization_key = NULL;
  cpu_affinity = -1;
  redis_host = NULL;
  redis_port = 6379;
  dns_mode = 0;
}

/* ******************************************* */

Prefs::~Prefs() {
  ;
}

/* ******************************************* */

void usage() {
  NetworkInterface n;

  printf("ntopng %s v.%s (%s) - (C) 1998-13 ntop.org\n\n"
	 "Usage:\n"
	 "  ntopng <configuration file>\n"
	 "  or\n"
	 "  ntopng -m <local nets> [-d <data dir>] [-n mode] [-i <iface>]\n"
	 "              [-w <http port>] [-p <protos>] [-d <path>]\n"
	 "              [-c <categorization key>] [-r <redis>]\n"
	 "              [-l] [-s] [-v]\n\n"
	 "Options:\n"
	 "[--dns-mode|-n] <mode>              | DNS address resolution mode\n"
	 "                                    | 0 - Decode DNS responses and resolve\n"
	 "                                    |     local numeric IPs only (default)\n"
	 "                                    | 1 - Decode DNS responses and resolve all\n"
	 "                                    |     numeric IPs\n"
	 "                                    | 2 - Decode DNS responses and don't\n"
	 "                                    |     resolve numeric IPs\n"
	 "                                    | 3 - Don't decode DNS responses and don't\n"
	 "                                    |     resolve numeric IPs\n"
	 "[--interface|-i] <interface>        | Input interface name (numeric or symbolic)\n"
	 "[--data-dir|-d] <path>              | Data directory (must be writable).\n"
	 "                                    | Default: %s\n"
	 "[--httpdocs-dir|-1] <path>          | Http documents root directory.\n"
	 "                                    | Default: %s\n"
	 "[--scripts-dir|-2] <path>           | Scripts directory.\n"
	 "                                    | Default: %s\n"
	 "[--callbacks-dir|-3] <path>         | Callbacks directory.\n"
	 "                                    | Default: %s\n"
	 "[--categorization-key|-c] <key>     | Key used to access host categorization\n"
	 "                                    | services (default: disabled). \n"
	 "                                    | Please read README.categorization for\n"
	 "                                    | more info.\n"
	 "[--http-port|-w] <http port>        | HTTP port. Default: %u\n"
	 "[--local-networks|-m] <local nets>  | List of local networks\n"
	 "                                    | (e.g. -m \"192.168.0.0/24,172.16.0.0/16\")\n"
	 "[--ndpi-protocols|-p] <file>.protos | Specify a nDPI protocol file\n"
	 "                                    | (eg. protos.txt)\n"
	 "[--redis|-r] <redis host[:port]>    | Redis host[:port]\n"
	 "[--core-affinity|-g] <cpu core id>  | Bind the capture/processing thread to a\n"
	 "                                    | specific CPU Core\n"
	 "[--dont-change-user|-s]             | Do not change user (debug only)\n"
	 "[--disable-login|-l]                | Disable user login authentication\n"
	 "[--users-file|-u] <path>            | Users configuration file path\n"
	 "                                    | Default: %s\n"
	 "[--verbose|-v]                      | Verbose tracing\n"
	 "[--help|-h]                         | Help\n"
	 , PACKAGE_MACHINE, PACKAGE_VERSION, PACKAGE_RELEASE, 
	 CONST_DEFAULT_DATA_DIR, CONST_DEFAULT_DOCS_DIR, CONST_DEFAULT_SCRIPTS_DIR,
         CONST_DEFAULT_CALLBACKS_DIR, CONST_DEFAULT_NTOP_PORT, CONST_DEFAULT_USERS_FILE);

  printf("\n");
  n.printAvailableInterfaces(true, 0, NULL, 0);

  exit(0);
}

/* ******************************************* */

static const struct option long_options[] = {
  { "dns-mode",                          required_argument, NULL, 'n' },
  { "interface",                         required_argument, NULL, 'i' },
  { "data-dir",                          required_argument, NULL, 'd' },
  { "categorization-key",                required_argument, NULL, 'c' },
  { "http-port",                         required_argument, NULL, 'w' },
  { "local-networks",                    required_argument, NULL, 'm' },
  { "ndpi-protocols",                    required_argument, NULL, 'p' },
  { "redis",                             required_argument, NULL, 'r' },
  { "core-affinity",                     required_argument, NULL, 'g' },
  { "dont-change-user",                  no_argument,       NULL, 's' },
  { "disable-login",                     no_argument,       NULL, 'l' },
  { "users-file",                        required_argument, NULL, 'u' },
  { "verbose",                           no_argument,       NULL, 'v' },
  { "help",                              no_argument,       NULL, 'h' },
  { "httpdocs-dir",                      required_argument, NULL, '1' },
  { "scripts-dir",                       required_argument, NULL, '2' },
  { "callbacks-dir",                     required_argument, NULL, '3' },
  /* End of options */
  { NULL,                                no_argument,       NULL,  0 }
};

/* ******************************************* */

int Prefs::setOption(int optkey, char *optarg) {
  switch(optkey) {
    case 'c':
      categorization_key = optarg;
      break;

    case 'g':
      cpu_affinity = atoi(optarg);
      break;

    case 'm':
      local_networks = strdup(optarg);
      ntop->setLocalNetworks(local_networks);
      localnets = true;
      break;

    case 'n':
      dns_mode = atoi(optarg);
      switch(dns_mode) {
      case 0:
	break;
      case 1:
	resolve_all_hosts();
	break;
      case 2:
	disable_dns_resolution();
	break;
      case 3:
	disable_dns_resolution();
	disable_dns_responses_decoding();
	break;
      default:
	help();
      }
      break;

    case 'p':
      ndpi_proto_path = strdup(optarg);
      ntop->setCustomnDPIProtos(ndpi_proto_path);
      break;

    case 'h':
      help();
      break;

    case 'i':
      ifName = optarg;
      break;

    case 'w':
      http_port = atoi(optarg);
      break;

    case 'r':
      {
	char buf[64];
	snprintf(buf, sizeof(buf), "%s", optarg);
	redis_host = strtok(buf, ":");
	if(redis_host) {
	  char *c = strtok(NULL, ":");
	  if(c) redis_port = atoi(c);
	}
      }
      break;

    case 's':
      change_user = false;
      break;

    case 'd':
      free(data_dir);
      data_dir = strdup(optarg);
      break;

    case '1':
      free(docs_dir);
      docs_dir = strdup(optarg);
      break;

    case '2':
      free(scripts_dir);
      scripts_dir = strdup(optarg);
      break;

    case '3':
      free(callbacks_dir);
      callbacks_dir = strdup(optarg);
      break;

    case 'l':
      enable_users_login = false;
      break;

    case 'u':
      free(users_file_path);
      users_file_path = strdup(optarg);
      break;

    case 'v':
      ntop->getTrace()->set_trace_level(MAX_TRACE_LEVEL);
      break;

    default:
      return(-1);
  }

  return(0);
}

/* ******************************************* */

int Prefs::checkOptions() {
  data_dir       = ntop->getValidPath(data_dir);
  docs_dir       = ntop->getValidPath(docs_dir);
  scripts_dir    = ntop->getValidPath(scripts_dir);
  callbacks_dir  = ntop->getValidPath(callbacks_dir);

  if(!data_dir)      { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate data dir"); return(-1);      }
  if(!docs_dir)      { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate docs dir"); return(-1);      }
  if(!scripts_dir)   { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate scripts dir"); return(-1);   }
  if(!callbacks_dir) { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate callbacks dir"); return(-1); }

  if(!localnets) help();

  return(0);
}

/* ******************************************* */

int Prefs::loadFromCLI(int argc, char *argv[]) {
  u_char c;

  while((c = getopt_long(argc, argv, "c:g:hi:w:r:sm:n:p:d:1:2:3:lvu:", long_options, NULL)) != '?') {
    if(c == 255) break;
    setOption(c, optarg);
  }

  return(checkOptions());
}

/* ******************************************* */

int Prefs::loadFromFile(const char *path) {
  char buffer[512], *line, *key, *value;
  FILE *fd;
  const struct option *opt;

  config_file_path = strdup(path);

  fd = fopen(config_file_path, "r");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Config file %s not found", config_file_path);
    return(-1);
  }

  while(fd) {
    if(!(line = fgets(buffer, sizeof(buffer), fd)))
      break;

    line = Utils::trim(line);

    if(strlen(line) < 1 || line[0] == '#')
      continue;

    key = line;
    key = Utils::trim(key);

    value = strrchr(line, '=');
    if(value == NULL)
      value = &line[strlen(line)]; /* empty */
    else
      value[0] = 0, value = &value[1];
    value = Utils::trim(value);

    opt = long_options;
    while (opt->name != NULL) {
      if (strcmp(opt->name, key) == 0 || 
          (strlen(key) == 1 && opt->val == key[0])) {
        setOption(opt->val, value);
        break;
      }
      opt++;
    }
  }

  fclose(fd);

  return(checkOptions());
}

/* ******************************************* */

int Prefs::save() {
  FILE *fd;

  saveUsersToFile();

  if (config_file_path == NULL)
    return(-1);

  fd = fopen(config_file_path, "w");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to open file %s [%s]", config_file_path, strerror(errno));
    return(-1);
  }

  if(dns_mode != 0)       fprintf(fd, "dns-mode=%d\n", dns_mode);
  if(ifName)              fprintf(fd, "interface=%s\n", ifName);
  if(data_dir)            fprintf(fd, "data-dir=%s\n", data_dir);
  if(categorization_key)  fprintf(fd, "categorization-key=%s\n", categorization_key);
  if(local_networks)      fprintf(fd, "local-networks=%s\n", local_networks);
  if(ndpi_proto_path)     fprintf(fd, "ndpi-protocols=%s\n", ndpi_proto_path);
  if(redis_host)          fprintf(fd, "redis=%s:%d\n", redis_host, redis_port);
  if(cpu_affinity >= 0)   fprintf(fd, "core-affinity=%d\n", cpu_affinity);
  if(!change_user)        fprintf(fd, "dont-change-user\n");
  if(!enable_users_login) fprintf(fd, "disable-login\n");
  if(users_file_path)     fprintf(fd, "users-file=%s\n", users_file_path);
  if(docs_dir)            fprintf(fd, "httpdocs-dir=%s\n", docs_dir);
  if(scripts_dir)         fprintf(fd, "scripts-dir=%s\n", scripts_dir);
  if(callbacks_dir)       fprintf(fd, "callbacks-dir=%s\n", callbacks_dir);
  if(http_port != CONST_DEFAULT_NTOP_PORT) fprintf(fd, "http-port=%d\n", http_port);
  if(ntop->getTrace()->get_trace_level() != TRACE_LEVEL_NORMAL) fprintf(fd, "verbose\n");

  fclose(fd);

  return(0);
}

/* ******************************************* */

int Prefs::loadUsersFromFile() {
  char buffer[512], *line, *key, *value;
  FILE *fd;
  int i;

  if (users_file_path == NULL)
    return(-1);

  fd = fopen(users_file_path, "r");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Config file %s not found (it will be created)", users_file_path);
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

    /* inserting all users info into redis */
    if (strncmp(key, "user.", 5) == 0) {
      if(ntop->getRedis()->set(key, value, 0) < 0)
        ntop->getTrace()->traceEvent(TRACE_WARNING, "Error setting '%s' = '%s'", key, value);
    }
  }

  fclose(fd);

  return(0);
}

/* ******************************************* */

int Prefs::saveUsersToFile() {
  char **keys;
  char val[64];
  int rc, i;
  FILE *fd;

  if (users_file_path == NULL)
    return(-1);

  fd = fopen(users_file_path, "w");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to open file %s [%s]", users_file_path, strerror(errno));
    return(-1);
  }

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

