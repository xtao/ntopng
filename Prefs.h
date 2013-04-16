/*
 *
 * (C) 2013 - prefs.org
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

class Prefs {
 private:
  bool enable_dns_resolution, sniff_dns_responses;

 public:
  Prefs();
  ~Prefs();

  inline void disable_dns_resolution()            { enable_dns_resolution = false; };
  inline bool is_dns_resolution_enabled()         { return(enable_dns_resolution); };
  inline void disable_dns_responses_decoding()    { sniff_dns_responses = false;   };
  inline bool decode_dns_responses()              { return(sniff_dns_responses);   };
};

#endif /* _PREFS_H_ */
