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
#include "ewah.h"

#define TIME_GRANULARITY          5 /* sec */
#define MAX_DURATION          86400 /* wrap every day */

/* *************************************** */

/* Daily duration */
ActivityStats::ActivityStats() {
  _bitset = new EWAHBoolArray<u_int32_t>;

  wrap_time = time(NULL);
  wrap_time -= (wrap_time % MAX_DURATION);
  wrap_time += MAX_DURATION;
  last_set_time = 0;
}

/* *************************************** */

ActivityStats::~ActivityStats() {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;

  delete bitset;
}

/* *************************************** */

void ActivityStats::reset() {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;
  bitset->reset();
  last_set_time = 0;
}

/* *************************************** */

void ActivityStats::set(time_t when) {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;

  if(when > wrap_time) {
    bitset->reset();
    wrap_time += MAX_DURATION;
  }

  when %= MAX_DURATION;

  if(when == last_set_time) return;
  
  m.lock(__FILE__, __LINE__);
  bitset->set((size_t)when);
  m.unlock(__FILE__, __LINE__);
  last_set_time = when;
};

/* *************************************** */

void ActivityStats::setDump(stringstream* dump) {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;

  bitset->read(*dump);
}

/* *************************************** */

bool ActivityStats::dump(char* path) {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;

  try {
    ofstream dumpFile(path);
    stringstream ss;
    
    m.lock(__FILE__, __LINE__);
    bitset->write(ss);
    m.unlock(__FILE__, __LINE__);

    dumpFile << ss.str();
    dumpFile.close();
    return(true);
  } catch(...) {
    return(false);
  }
}

/* *************************************** */

json_object* ActivityStats::getJSONObject() {
  json_object *my_object;
  char buf[32];
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;
  time_t begin_time = wrap_time - MAX_DURATION;

  my_object = json_object_new_object();

  m.lock(__FILE__, __LINE__);
  for(EWAHBoolArray<u_int32_t>::const_iterator i = bitset->begin(); i != bitset->end(); ++i) {
    snprintf(buf, sizeof(buf), "%lu", (begin_time+(*i)));
    json_object_object_add(my_object, buf, json_object_new_int(1));
  }
  m.unlock(__FILE__, __LINE__);

  return(my_object);
}

/* *************************************** */

char* ActivityStats::serialize() {
  json_object *my_object = getJSONObject();
  char *rsp = strdup(json_object_to_json_string(my_object));

  /* Free memory */
  json_object_put(my_object);

  return(rsp);
}

/* *************************************** */

void ActivityStats::deserialize(json_object *o) {
  EWAHBoolArray<u_int32_t> *bitset = (EWAHBoolArray<u_int32_t>*)_bitset;
  struct json_object_iterator it, itEnd;

  if(!o) return;

  /* Reset all */
  bitset->reset();

  it = json_object_iter_begin(o), itEnd = json_object_iter_end(o);
  
  while (!json_object_iter_equal(&it, &itEnd)) {
    char *key  = (char*)json_object_iter_peek_name(&it);
    u_int32_t when = atol(key);
      
    when %= MAX_DURATION;
    bitset->set(when);
    // ntop->getTrace()->traceEvent(TRACE_WARNING, "%s=%d", key, 1);
    
    json_object_iter_next(&it);
  }
}
