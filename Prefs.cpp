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

/* ******************************************* */

Prefs::Prefs(Ntop *_ntop) {
  num_deferred_interfaces_to_register = 0;
  memset(deferred_interfaces_to_register, 0, sizeof(deferred_interfaces_to_register));
  ntop = _ntop, dump_timeline = false, sticky_hosts = location_none;
  local_networks = strdup(CONST_DEFAULT_HOME_NET","CONST_DEFAULT_LOCAL_NETS);
  enable_dns_resolution = sniff_dns_responses = true;
  categorization_enabled = false, httpbl_enabled = false, resolve_all_host_ip = false;
  max_num_hosts = MAX_NUM_INTERFACE_HOSTS, max_num_flows = MAX_NUM_INTERFACE_HOSTS;
  data_dir = strdup(CONST_DEFAULT_DATA_DIR);
  docs_dir = strdup(CONST_DEFAULT_DOCS_DIR);
  scripts_dir = strdup(CONST_DEFAULT_SCRIPTS_DIR);
  callbacks_dir = strdup(CONST_DEFAULT_CALLBACKS_DIR);
  config_file_path = ndpi_proto_path = NULL;
  http_port = CONST_DEFAULT_NTOP_PORT;
  http_prefix = strdup("");
  https_port = CONST_DEFAULT_NTOP_PORT+1;
  change_user = true, daemonize = false;
  user = strdup(CONST_DEFAULT_NTOP_USER);
  categorization_key = NULL;
  httpbl_key = NULL;
  cpu_affinity = -1;
  redis_host = strdup("127.0.0.1");
  redis_port = 6379;
  dns_mode = 0;
  logFd = NULL;
  disable_alerts = false;
  pid_path = strdup(DEFAULT_PID_PATH);
  packet_filter = NULL;
  disable_host_persistency = false;
  num_interfaces = 0, enable_auto_logout = true;
  dump_flows_on_db = false;
  enable_aggregations = aggregations_disabled;
  memset(ifNames, 0, sizeof(ifNames));
  dump_hosts_to_db = location_none, dump_aggregations_to_db = location_none;
  shorten_aggregation_names = true; // TODO: make it configurable
  json_symbolic_labels = 0;
#ifdef WIN32
  daemonize = true;
#endif
  export_endpoint = NULL;
  enable_ixia_timestamps = false;

  /* Defaults */
  non_local_host_max_idle = MAX_REMOTE_HOST_IDLE /* sec */;
  local_host_max_idle     = MAX_LOCAL_HOST_IDLE /* sec */;
  flow_max_idle           = MAX_FLOW_IDLE /* sec */;
  host_max_new_flows_sec_threshold = CONST_MAX_NEW_FLOWS_SECOND; /* flows/sec */
  host_max_num_syn_sec_threshold = CONST_MAX_NUM_SYN_PER_SECOND; /* syn/sec */
}

/* ******************************************* */

Prefs::~Prefs() {
  for(int i=0; i<num_deferred_interfaces_to_register; i++)
    if(deferred_interfaces_to_register[i] != NULL)
      free(deferred_interfaces_to_register[i]);

  if(logFd) fclose(logFd);
  if(data_dir) free(data_dir);
  if(docs_dir) free(docs_dir);
  if(scripts_dir) free(scripts_dir);
  if(callbacks_dir) free(callbacks_dir);
  if(config_file_path) free(config_file_path);
  if(user) free(user);
  if(pid_path) free(pid_path);
  if(packet_filter) free(packet_filter);
  free(http_prefix);
}

/* ******************************************* */

/* C-binding needed by Win32 service call */
void usage() {
  NetworkInterface n("dummy");

  printf("ntopng %s v.%s (%s) - "NTOP_COPYRIGHT"\n\n"
	 "Usage:\n"
	 "  ntopng <configuration file path>\n"
	 "  or\n"
	 "  ntopng <command line options> \n\n"
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
	 "[--interface|-i] <interface|pcap>   | Input interface name (numeric/symbolic)\n"
	 "                                    | or pcap file path\n"
#ifndef WIN32
	 "[--data-dir|-d] <path>              | Data directory (must be writable).\n"
	 "                                    | Default: %s\n"
	 "[--daemon|-e]                       | Daemonize ntopng\n"
#endif
	 "[--httpdocs-dir|-1] <path>          | HTTP documents root directory.\n"
	 "                                    | Default: %s\n"
	 "[--scripts-dir|-2] <path>           | Scripts directory.\n"
	 "                                    | Default: %s\n"
	 "[--callbacks-dir|-3] <path>         | Callbacks directory.\n"
	 "                                    | Default: %s\n"
	 "[--dump-timeline|-C]                | Enable timeline dump.\n"
	 "[--categorization-key|-c] <key>     | Key used to access host categorization\n"
	 "                                    | services (default: disabled). \n"
	 "                                    | Please read README.categorization for\n"
	 "                                    | more info.\n"
	 "[--httpbl-key|-k] <key>             | Key used to access httpbl\n"
	 "                                    | services (default: disabled). \n"
	 "                                    | Please read README.httpbl for\n"
	 "                                    | more info.\n"
	 "[--http-port|-w] <http port>        | HTTP port. Set to 0 to disable http server. Default: %u\n"
	 "[--https-port|-W] <http port>       | HTTPS port. Default: %u\n"
	 "[--local-networks|-m] <local nets>  | Local nets list (default: 192.168.1.0/24)\n"
	 "                                    | (e.g. -m \"192.168.0.0/24,172.16.0.0/16\")\n"
	 "[--ndpi-protocols|-p] <file>.protos | Specify a nDPI protocol file\n"
	 "                                    | (eg. protos.txt)\n"
	 "[--disable-host-persistency|-P]     | Disable host persistency\n"
	 "[--redis|-r] <redis host[:port]>    | Redis host[:port]\n"
#ifdef linux
	 "[--core-affinity|-g] <cpu core id>  | Bind the capture/processing thread to a\n"
	 "                                    | specific CPU Core\n"
#endif
	 "[--user|-U] <sys user>              | Run ntopng with the specified user\n"
	 "                                    | instead of %s\n"
	 "[--dont-change-user|-s]             | Do not change user (debug only)\n"
	 "[--disable-autologout|-q]           | Disable web interface logout for inactivity\n"
	 "[--disable-login|-l]                | Disable user login authentication\n"
	 "[--max-num-flows|-X] <num>          | Max number of active flows\n"
	 "                                    | (default: %u)\n"
	 "[--max-num-hosts|-x] <num>          | Max number of active hosts\n"
	 "                                    | (default: %u)\n"
	 "[--users-file|-u] <path>            | Users configuration file path\n"
	 "                                    | Default: %s\n"
#ifndef WIN32
	 "[--pid|-G] <path>                   | Pid file path\n"
#endif

	 "[--disable-alerts|-H]               | Disable alerts generation\n"
	 "[--packet-filter|-B] <filter>       | Ingress packet filter (BPF filter)\n"
	 "[--enable-aggregations|-A] <mode>   | Setup data aggregation:\n"
	 "                                    | 0 - No aggregations (default)\n"
	 "                                    | 1 - Enable aggregations, no timeline dump\n"
	 "                                    | 2 - Enable aggregations, with timeline\n"
	 "                                    |     dump (see -C)\n"
	 "[--dump-flows|-F]                   | Dump expired flows.\n"
	 "[--export-flows|-I] <endpoint>      | Export flows using the specified endpoint.\n"
	 "[--dump-hosts|-D] <mode>            | Dump hosts policy (default: none).\n"
	 "                                    | Values:\n"
	 "                                    | all    - Dump all hosts\n"
	 "                                    | local  - Dump only local hosts\n"
	 "                                    | remote - Dump only remote hosts\n"
	 "[--dump-aggregations|-E] <mode>     | Dump aggregations policy (default: none).\n"
	 "                                    | Values:\n"
	 "                                    | all    - Dump all hosts\n"
	 "                                    | local  - Dump only local hosts\n"
	 "                                    | remote - Dump only remote hosts\n"
	 "[--sticky-hosts|-S] <mode>          | Don't flush hosts (default: none).\n"
	 "                                    | Values:\n"
	 "                                    | all    - Keep all hosts in memory\n"
	 "                                    | local  - Keep only local hosts\n"
	 "                                    | remote - Keep only remote hosts\n"
	 "                                    | none   - Flush hosts when idle\n"
	 "--json-labels                       | In case JSON label is used (e.g. with\n"
	 "                                    | ZMQ/Sqlite) labels instead of numbers are\n"
	 "                                    | used as keys.\n"
	 "--hw-timestamp-mode <mode>          | Enable hw timestamping/stripping.\n"
	 "                                    | Supported TS modes are:\n"
	 "                                    | ixia - Timestamped packets by ixiacom.com\n"
	 "                                    |        hardware devices\n"
	 "[--http-prefix|-Z] <prefix>         | HTTP prefix to be prepended to URLs. This is\n"
	 "                                    | useful when using ntopng behind a proxy.\n"
	 "[--verbose|-v]                      | Verbose tracing\n"
	 "[--version|-V]                      | Print version and quit\n"
	 "[--help|-h]                         | Help\n"
	 , PACKAGE_MACHINE, PACKAGE_VERSION, NTOPNG_SVN_RELEASE,
#ifndef WIN32
	 ntop->get_working_dir(),
#endif
	 CONST_DEFAULT_DOCS_DIR, CONST_DEFAULT_SCRIPTS_DIR,
         CONST_DEFAULT_CALLBACKS_DIR, CONST_DEFAULT_NTOP_PORT, CONST_DEFAULT_NTOP_PORT+1,
         CONST_DEFAULT_NTOP_USER,
	 MAX_NUM_INTERFACE_HOSTS, MAX_NUM_INTERFACE_HOSTS/2,
	 CONST_DEFAULT_USERS_FILE);

  printf("\n");
  n.printAvailableInterfaces(true, 0, NULL, 0);

  _exit(0);
}

/* ******************************************* */

u_int32_t Prefs::getDefaultPrefsValue(const char *pref_key, u_int32_t default_value) {
  char rsp[32];

  if(ntop->getRedis()->get((char*)pref_key, rsp, sizeof(rsp)) == 0)
    return(atoi(rsp));
  else {
    snprintf(rsp, sizeof(rsp), "%u", default_value);
    ntop->getRedis()->set((char*)pref_key, rsp);
    return(default_value);
  }
}

/* ******************************************* */

void Prefs::loadIdleDefaults() {
  local_host_max_idle = getDefaultPrefsValue(CONST_LOCAL_HOST_IDLE_PREFS, MAX_LOCAL_HOST_IDLE);
  non_local_host_max_idle = getDefaultPrefsValue(CONST_REMOTE_HOST_IDLE_PREFS, MAX_REMOTE_HOST_IDLE);
  flow_max_idle = getDefaultPrefsValue(CONST_FLOW_MAX_IDLE_PREFS, MAX_FLOW_IDLE);
  host_max_new_flows_sec_threshold = getDefaultPrefsValue(CONST_MAX_NEW_FLOWS_PREFS, CONST_MAX_NEW_FLOWS_SECOND);
  host_max_num_syn_sec_threshold = getDefaultPrefsValue(CONST_MAX_NUM_SYN_PREFS, CONST_MAX_NUM_SYN_PER_SECOND);
}

/* ******************************************* */

static const struct option long_options[] = {
  { "dns-mode",                          required_argument, NULL, 'n' },
  { "interface",                         required_argument, NULL, 'i' },
#ifndef WIN32
  { "data-dir",                          required_argument, NULL, 'd' },
#endif
  { "categorization-key",                required_argument, NULL, 'c' },
  { "httpbl-key",                        required_argument, NULL, 'k' },
  { "daemon",                            no_argument,       NULL, 'e' },
  { "core-affinity",                     required_argument, NULL, 'g' },
  { "help",                              no_argument,       NULL, 'h' },
  { "disable-login",                     no_argument,       NULL, 'l' },
  { "local-networks",                    required_argument, NULL, 'm' },
  { "ndpi-protocols",                    required_argument, NULL, 'p' },
  { "disable-autologout",                no_argument,       NULL, 'q' },
  { "redis",                             required_argument, NULL, 'r' },
  { "dont-change-user",                  no_argument,       NULL, 's' },
  { "verbose",                           no_argument,       NULL, 'v' },
  { "max-num-hosts",                     required_argument, NULL, 'x' },
  { "http-port",                         required_argument, NULL, 'w' },
  { "enable-aggregations",               no_argument,       NULL, 'A' },
  { "packet-filter",                     required_argument, NULL, 'B' },
  { "dump-timeline",                     no_argument,       NULL, 'C' },
  { "dump-hosts",                        required_argument, NULL, 'D' },
  { "dump-aggregations",                 required_argument, NULL, 'E' },
  { "dump-flows",                        no_argument,       NULL, 'F' },
#ifndef WIN32
  { "pid",                               required_argument, NULL, 'G' },
#endif
  { "disable-alerts",                    no_argument,       NULL, 'H' },
  { "export-flows",                      required_argument, NULL, 'I' },
  { "disable-host-persistency",          no_argument,       NULL, 'P' },
  { "sticky-hosts",                      required_argument, NULL, 'S' },
  { "user",                              required_argument, NULL, 'U' },
  { "version",                           no_argument,       NULL, 'V' },
  { "max-num-flows",                     required_argument, NULL, 'X' },
  { "https-port",                        required_argument, NULL, 'W' },
  { "http-prefix",                       required_argument, NULL, 'Z' },
  { "httpdocs-dir",                      required_argument, NULL, '1' },
  { "scripts-dir",                       required_argument, NULL, '2' },
  { "callbacks-dir",                     required_argument, NULL, '3' },
  { "json-labels",                       no_argument,       NULL, 211 },
  { "hw-timestamp-mode",                 required_argument, NULL, 212 },

  /* End of options */
  { NULL,                                no_argument,       NULL,  0 }
};

/* ******************************************* */

int Prefs::setOption(int optkey, char *optarg) {
  switch(optkey) {
  case 'A':
    switch(atoi(optarg)) {
    case 0:
      enable_aggregations = aggregations_disabled;
      break;
    case 1:
      enable_aggregations = aggregations_enabled_no_bitmap_dump;
      break;
    case 2:
      enable_aggregations = aggregations_enabled_with_bitmap_dump;
      break;
    default:
      ntop->getTrace()->traceEvent(TRACE_ERROR, "Invalid value for -A: disabling aggregations");
      enable_aggregations = aggregations_disabled;
      break;
    }
    break;

  case 'B':
    if((optarg[0] == '\"') && (strlen(optarg) > 2)) {
      packet_filter = strdup(&optarg[1]);
      packet_filter[strlen(packet_filter)-1] = '\0';
    } else
      packet_filter = strdup(optarg);
    break;

  case 'c':
    categorization_key = optarg;
    break;

  case 'k':
    httpbl_key = optarg;
    break;

  case 'C':
    dump_timeline = true;
    break;

#ifndef WIN32
  case 'd':
    ntop->setWorkingDir(optarg);
    break;
#endif

  case 'D':
    if(!strcmp(optarg, "all")) dump_hosts_to_db = location_all;
    else if(!strcmp(optarg, "local")) dump_hosts_to_db = location_local_only;
    else if(!strcmp(optarg, "remote")) dump_hosts_to_db = location_remote_only;
    else if(!strcmp(optarg, "none")) dump_hosts_to_db = location_none;
    else ntop->getTrace()->traceEvent(TRACE_ERROR, "Unknown value %s for -D", optarg);
    break;

  case 'e':
    daemonize = true;
    break;

  case 'E':
    if(!strcmp(optarg, "all")) dump_aggregations_to_db = location_all;
    else if(!strcmp(optarg, "local")) dump_aggregations_to_db = location_local_only;
    else if(!strcmp(optarg, "remote")) dump_aggregations_to_db = location_remote_only;
    else if(!strcmp(optarg, "none")) dump_aggregations_to_db = location_none;
    else ntop->getTrace()->traceEvent(TRACE_ERROR, "Unknown value %s for -E", optarg);
    break;

  case 'S':
    if(!strcmp(optarg, "all")) sticky_hosts = location_all;
    else if(!strcmp(optarg, "local")) sticky_hosts = location_local_only;
    else if(!strcmp(optarg, "remote")) sticky_hosts = location_remote_only;
    else if(!strcmp(optarg, "none")) sticky_hosts = location_none;
    else ntop->getTrace()->traceEvent(TRACE_ERROR, "Unknown value %s for -S", optarg);
    break;
  case 'g':
    cpu_affinity = atoi(optarg);
    break;

  case 'm':
    free(local_networks);
    local_networks = strdup(optarg);
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

  case 'q':
    enable_auto_logout = false;
    break;

  case 'P':
    disable_host_persistency = true;
    break;

  case 'h':
    help();
    break;

  case 'i':
    if(num_deferred_interfaces_to_register < MAX_NUM_INTERFACES)
      deferred_interfaces_to_register[num_deferred_interfaces_to_register++] = strdup(optarg);
    else
      ntop->getTrace()->traceEvent(TRACE_WARNING, "Too many interfaces specified with -i: ignored %s", optarg);
    break;

  case 'w':
    http_port = atoi(optarg);
    break;

  case 'W':
    https_port = atoi(optarg);
    break;

  case 'Z':
    if(optarg[0] != '/') {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "-Z argument (%s) must begin with '/' (example /ntopng): skipped", optarg);
    } else {
      int len = strlen(optarg);

      if(len > 0) {
	if(optarg[len-1] == '/') {
	  ntop->getTrace()->traceEvent(TRACE_WARNING, "-Z argument (%s) cannot end with '/' (example /ntopng): skipped", optarg);
	} else {
	  free(http_prefix);
	  http_prefix = strdup(optarg);
	}
      }
    }
    break;

  case 'r':
    {
      char buf[64], *r;

      snprintf(buf, sizeof(buf), "%s", optarg);
      r = strtok(buf, ":");
      if(r) {
	char *c = strtok(NULL, ":");
	if(c) redis_port = atoi(c);

	if(redis_host) free(redis_host);
	redis_host = strdup(r);
      }
    }
    break;

  case 's':
    change_user = false;
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

  case 'x':
    max_num_hosts = max_val(atoi(optarg), 1024);
    break;

  case 'v':
    ntop->getTrace()->set_trace_level(MAX_TRACE_LEVEL);
    break;

  case 'F':
    dump_flows_on_db = true;
    break;

#ifndef WIN32
  case 'G':
    pid_path = strdup(optarg);
    break;
#endif

  case 'H':
    disable_alerts = true;
    break;

  case 'I':
    export_endpoint = strdup(optarg);
    break;

  case 'U':
    free(user);
    user = strdup(optarg);
    break;

  case 'V':
    printf("v.%s (%s)\n", PACKAGE_VERSION, NTOPNG_SVN_RELEASE);
    _exit(0);
    break;

  case 'X':
    max_num_flows = max_val(atoi(optarg), 1024);
    break;

  case 211:
    json_symbolic_labels = 1;
    break;

  case 212:
    if(strcmp(optarg, "ixia") == 0)
      enable_ixia_timestamps = true;
    else
      ntop->getTrace()->traceEvent(TRACE_WARNING,
				   "Unknown --hw-timestamp-mode mode, it has been ignored\n");
    break;

  default:
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Unknown option -%c: Ignored.", (char)optkey);
    return(-1);
  }

  return(0);
}

/* ******************************************* */

int Prefs::checkOptions() {
#ifndef WIN32
  if(daemonize)
#endif
    {
      char path[MAX_PATH];

      ntop_mkdir(data_dir, 0777);
      snprintf(path, sizeof(path), "%s/ntopng.log", ntop->get_working_dir() /* "C:\\Windows\\Temp" */);
      ntop->fixPath(path);
      logFd = fopen(path, "w");
      if(logFd)
	ntop->getTrace()->traceEvent(TRACE_NORMAL, "Logging into %s", path);
      else
	ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to create log %s", path);
    }

  free(data_dir); data_dir = strdup(ntop->get_install_dir());
  docs_dir      = ntop->getValidPath(docs_dir);
  scripts_dir   = ntop->getValidPath(scripts_dir);
  callbacks_dir = ntop->getValidPath(callbacks_dir);

  if(!data_dir)         { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate data dir");      return(-1); }
  if(!docs_dir[0])      { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate docs dir");      return(-1); }
  if(!scripts_dir[0])   { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate scripts dir");   return(-1); }
  if(!callbacks_dir[0]) { ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to locate callbacks dir"); return(-1); }

  ntop->removeTrailingSlash(docs_dir);
  ntop->removeTrailingSlash(scripts_dir);
  ntop->removeTrailingSlash(callbacks_dir);

  return(0);
}

/* ******************************************* */

int Prefs::loadFromCLI(int argc, char *argv[]) {
  u_char c;

  while((c = getopt_long(argc, argv,
			 "c:k:eg:hi:w:r:sg:m:n:p:qd:x:1:2:3:lvA:B:CD:E:FG:HLI:S:U:X:W:VZ:",
			 long_options, NULL)) != '?') {
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

  while(true) {
    if(!(line = fgets(buffer, sizeof(buffer), fd)))
      break;

    line = Utils::trim(line);

    if(strlen(line) < 1 || line[0] == '#')
      continue;

    key = line;
    key = Utils::trim(key);

    value = strrchr(line, '=');

    /* Fallback to space */
    if(value == NULL) value = strrchr(line, ' ');

    if(value == NULL)
      value = &line[strlen(line)]; /* empty */
    else
      value[0] = 0, value = &value[1];
    value = Utils::trim(value);

    if(strlen(key) > 2) key = &key[2];
    else key = &key[1];

    opt = long_options;
    while (opt->name != NULL) {
      if((strcmp(opt->name, key) == 0)
	 || ((key[1] == '\0') && (opt->val == key[0]))) {
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

  if(config_file_path == NULL)
    return(-1);

  fd = fopen(config_file_path, "w");

  if(fd == NULL) {
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Unable to open file %s [%s]", config_file_path, strerror(errno));
    return(-1);
  }

  if(dns_mode != 0)       fprintf(fd, "dns-mode=%d\n", dns_mode);

  if(num_interfaces > 0) {
    fprintf(fd, "interface=");

    for(int i=0; i<num_interfaces; i++)
      fprintf(fd, "%s%s", (i > 0) ? "," : "", ifNames[i].name);

    fprintf(fd, "\n");
  }
  if(data_dir)            fprintf(fd, "data-dir=%s\n", data_dir);
  if(categorization_key)  fprintf(fd, "categorization-key=%s\n", categorization_key);
  if(httpbl_key)          fprintf(fd, "httpbl-key=%s\n", httpbl_key);
  if(local_networks)      fprintf(fd, "local-networks=%s\n", local_networks);
  if(ndpi_proto_path)     fprintf(fd, "ndpi-protocols=%s\n", ndpi_proto_path);
  if(redis_host)          fprintf(fd, "redis=%s:%d\n", redis_host, redis_port);
  if(cpu_affinity >= 0)   fprintf(fd, "core-affinity=%d\n", cpu_affinity);
  if(!change_user)        fprintf(fd, "dont-change-user\n");
  if(!enable_users_login) fprintf(fd, "disable-login\n");
  if(docs_dir)            fprintf(fd, "httpdocs-dir=%s\n", docs_dir);
  if(scripts_dir)         fprintf(fd, "scripts-dir=%s\n", scripts_dir);
  if(callbacks_dir)       fprintf(fd, "callbacks-dir=%s\n", callbacks_dir);
  if(http_port != CONST_DEFAULT_NTOP_PORT) fprintf(fd, "http-port=%d\n", http_port);
  if(ntop->getTrace()->get_trace_level() != TRACE_LEVEL_NORMAL) fprintf(fd, "verbose\n");

  fclose(fd);

  return(0);
}

/* ******************************************* */

void Prefs::add_network_interface(char *name, char *description) {
  u_int32_t id = Utils::ifname2id(name);

  if(id < (MAX_NUM_INTERFACES-1)) {
    ifNames[id].name = strdup(name);
    ifNames[id].description = strdup(description ? description : name);
    num_interfaces++;
  } else
    ntop->getTrace()->traceEvent(TRACE_ERROR, "Too many interfaces: discarded %s", name);
}

/* ******************************************* */

void Prefs::add_default_interfaces() {
  NetworkInterface *dummy = new NetworkInterface("dummy");
  dummy->addAllAvailableInterfaces();
  delete dummy;
};

/* *************************************** */

void Prefs::lua(lua_State* vm) {
  lua_newtable(vm);

  lua_push_bool_table_entry(vm, "is_dns_resolution_enabled_for_all_hosts", resolve_all_host_ip);
  lua_push_bool_table_entry(vm, "is_dns_resolution_enabled", enable_dns_resolution);
  lua_push_bool_table_entry(vm, "is_categorization_enabled", categorization_enabled);
  lua_push_bool_table_entry(vm, "is_httpbl_enabled", httpbl_enabled);
  lua_push_int_table_entry(vm, "local_host_max_idle", local_host_max_idle);
  lua_push_int_table_entry(vm, "non_local_host_max_idle", non_local_host_max_idle);
  lua_push_int_table_entry(vm, "flow_max_idle", flow_max_idle);
  lua_push_int_table_entry(vm, "max_num_hosts", max_num_hosts);
  lua_push_int_table_entry(vm, "max_num_flows", max_num_flows);
  lua_push_bool_table_entry(vm, "is_dump_flows_enabled", dump_flows_on_db);
  lua_push_int_table_entry(vm, "dump_hosts", dump_hosts_to_db);
  lua_push_int_table_entry(vm, "dump_aggregation", dump_aggregations_to_db);
  lua_push_int_table_entry(vm, "host_max_new_flows_sec_threshold", host_max_new_flows_sec_threshold);
  lua_push_int_table_entry(vm, "host_max_num_syn_sec_threshold", host_max_num_syn_sec_threshold);
}

/* *************************************** */

void Prefs::registerNetworkInterfaces() {
  for(int i=0; i<num_deferred_interfaces_to_register; i++) {
    if(deferred_interfaces_to_register[i] != NULL)
      add_network_interface(deferred_interfaces_to_register[i], NULL);
  }
}
