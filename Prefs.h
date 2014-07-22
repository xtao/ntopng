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

#ifndef _PREFS_H_
#define _PREFS_H_

#include "ntop_includes.h"

class Ntop;

extern void usage();

class Prefs {
 private:
  Ntop *ntop;
  bool enable_dns_resolution, sniff_dns_responses, disable_host_persistency,
    categorization_enabled, httpbl_enabled, resolve_all_host_ip, change_user, daemonize,
    dump_timeline, shorten_aggregation_names, enable_auto_logout,
    disable_alerts;
  LocationPolicy dump_hosts_to_db, dump_aggregations_to_db, sticky_hosts;
  u_int16_t non_local_host_max_idle, local_host_max_idle, flow_max_idle;
  u_int32_t max_num_hosts, max_num_flows;
  u_int http_port, https_port;
  u_int8_t num_interfaces;
  bool dump_flows_on_db;
  AggregationMode enable_aggregations;
  char *ifNames[MAX_NUM_INTERFACES], *local_networks;
  char *data_dir, *docs_dir, *scripts_dir, *callbacks_dir, *export_endpoint;
  char *categorization_key;
  char *httpbl_key;
  char *config_file_path, *ndpi_proto_path;
  char *packet_filter;
  char *user;
  char *redis_host;
  char *pid_path;
  int redis_port;
  int cpu_affinity;
  int dns_mode;
  int json_symbolic_labels;
  FILE *logFd;


  inline void help() { usage(); };
  int setOption(int optkey, char *optarg);
  int checkOptions();

 public:
  Prefs(Ntop *_ntop);
  ~Prefs();

  inline char* get_local_networks()                     { return(local_networks);         };
  inline FILE* get_log_fd()                             { return(logFd);                  };
  inline LocationPolicy get_host_stickness()            { return(sticky_hosts);           };
  inline bool do_dump_timeline()                        { return(dump_timeline);          };
  inline void disable_dns_resolution()                  { enable_dns_resolution = false;  };
  inline void resolve_all_hosts()                       { resolve_all_host_ip = true;     };
  inline bool is_dns_resolution_enabled_for_all_hosts() { return(resolve_all_host_ip);    };
  inline bool is_dns_resolution_enabled()               { return(enable_dns_resolution);  };
  inline void disable_dns_responses_decoding()          { sniff_dns_responses = false;    };
  inline bool decode_dns_responses()                    { return(sniff_dns_responses);    };
  inline void enable_categorization()                   { categorization_enabled = true;  };
  inline void enable_httpbl()                           { httpbl_enabled = true;  };
  inline bool is_categorization_enabled()               { return(categorization_enabled); };
  inline bool is_httpbl_enabled()                       { return(httpbl_enabled); };
  inline bool do_change_user()                          { return(change_user);            };
  inline char* get_user()                               { return(user);                   };
  inline AggregationMode get_aggregation_mode()         { return(enable_aggregations);    };
  inline u_int8_t get_num_user_specified_interfaces()   { return(num_interfaces);         };
  inline bool  do_dump_flows_on_db()                    { return(dump_flows_on_db);       };
  inline char* get_if_name(u_int id)                    { return((id < MAX_NUM_INTERFACES) ? ifNames[id] : NULL); };
  inline char* get_data_dir()                           { return(data_dir);       };
  inline char* get_docs_dir()                           { return(docs_dir);       }; // HTTP docs
  inline char* get_scripts_dir()                        { return(scripts_dir);    };
  inline char* get_callbacks_dir()                      { return(callbacks_dir);  };
  inline char* get_export_endpoint()                    { return(export_endpoint);};
  inline char* get_categorization_key()                 { return(categorization_key); };
  inline char* get_httpbl_key()                         { return(httpbl_key); };
  inline bool  are_alerts_disabled()                    { return(disable_alerts);     };
  inline bool  is_host_persistency_enabled()            { return(disable_host_persistency ? false : true); };
  inline bool  use_short_aggregation_names()            { return(shorten_aggregation_names); };
  inline bool  do_auto_logout()                         { return(enable_auto_logout);        };
  inline int get_cpu_affinity()                         { return(cpu_affinity);   };
  inline u_int get_http_port()                          { return(http_port);      };
  inline u_int get_https_port()                         { return(https_port);     };
  inline char* get_redis_host()                         { return(redis_host);     }
  inline u_int get_redis_port()                         { return(redis_port);     };
  inline char* get_pid_path()                           { return(pid_path);       };
  inline char* get_packet_filter()                      { return(packet_filter);  };
  inline u_int16_t get_host_max_idle(bool localHost)    { return(localHost ? local_host_max_idle : non_local_host_max_idle);  };
  inline u_int16_t get_flow_max_idle()                  { return(flow_max_idle);  };
  inline u_int32_t get_max_num_hosts()                  { return(max_num_hosts);  };
  inline u_int32_t get_max_num_flows()                  { return(max_num_flows);  };
  inline bool daemonize_ntopng()                        { return(daemonize);                 };
  void add_default_interfaces();
  int loadFromCLI(int argc, char *argv[]);
  int loadFromFile(const char *path);
  inline void set_dump_hosts_to_db_policy(LocationPolicy p)   { dump_hosts_to_db = p;        };
  inline void get_dump_aggregations_to_db(LocationPolicy p)   { dump_aggregations_to_db = p; };
  inline LocationPolicy get_dump_hosts_to_db_policy()   { return(dump_hosts_to_db);          };
  inline LocationPolicy get_dump_aggregations_to_db()   { return(dump_aggregations_to_db);   };
  int save();
  void add_network_interface(char *name);
  inline u_int32_t get_json_symbolic_labels()                  { return(json_symbolic_labels);  };
  void lua(lua_State* vm);

};

#endif /* _PREFS_H_ */
