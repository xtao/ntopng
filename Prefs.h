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

#ifndef _PREFS_H_
#define _PREFS_H_

#include "ntop_includes.h"

class Ntop;

class Prefs {
 private:
  Ntop *ntop;
  bool enable_dns_resolution, sniff_dns_responses, categorization_enabled, resolve_all_host_ip, change_user, localnets;
  u_int16_t host_max_idle, flow_max_idle;
  u_int32_t max_num_hosts, max_num_flows;
  u_int http_port;
  char *ifName;
  char *data_dir, *docs_dir, *scripts_dir, *callbacks_dir;
  char *categorization_key;
  char *users_file_path, *config_file_path;
  char *redis_host;
  int redis_port;
  int cpu_affinity;

  void help();
  int setOption(int optkey, char *optarg);
  int checkOptions();
  int saveUsersToFile();

 public:
  Prefs(Ntop *_ntop);
  ~Prefs();

  inline void disable_dns_resolution()                  { enable_dns_resolution = false;  };
  inline void resolve_all_hosts()                       { resolve_all_host_ip = true;     };
  inline bool is_dns_resolution_enabled_for_all_hosts() { return(resolve_all_host_ip);    };
  inline bool is_dns_resolution_enabled()               { return(enable_dns_resolution);  };
  inline void disable_dns_responses_decoding()          { sniff_dns_responses = false;    };
  inline bool decode_dns_responses()                    { return(sniff_dns_responses);    };
  inline void enable_categorization()                   { categorization_enabled = true;  };
  inline bool is_categorization_enabled()               { return(categorization_enabled); };
  inline bool do_change_user()                          { return change_user; }
  inline char* get_if_name()                            { return(ifName); }
  inline char* get_data_dir()                           { return(data_dir); };
  inline char* get_docs_dir()                           { return(docs_dir); };
  inline char* get_scripts_dir()                        { return(scripts_dir); };
  inline char* get_callbacks_dir()                      { return(callbacks_dir); };
  inline char* get_categorization_key()                 { return(categorization_key); };
  inline int get_cpu_affinity()                         { return(cpu_affinity); };
  inline u_int get_http_port()                          { return(http_port); };
  inline char* get_redis_host()                         { return(redis_host); }
  inline u_int get_redis_port()                         { return(redis_port); };

  inline u_int16_t get_host_max_idle()                  { return(host_max_idle);          };
  inline u_int16_t get_flow_max_idle()                  { return(flow_max_idle);          };
  inline u_int32_t get_max_num_hosts()                  { return(max_num_hosts);          };
  inline u_int32_t get_max_num_flows()                  { return(max_num_flows);          };

  int loadUsersFromFile();
  int loadFromCLI(int argc, char *argv[]);
  int loadFromFile(const char *path);

  int save();
};

#endif /* _PREFS_H_ */
