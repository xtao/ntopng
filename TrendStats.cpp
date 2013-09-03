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

/* *************************************** */

TrendStats::TrendStats(time_t _begin_time, u_int _duration_sec) {
  begin_time = begin_time - (begin_time % 86400);
  end_time = begin_time + _duration_sec;
  _bitset = new EWAHBoolArray<uint32_t>;
}

/* *************************************** */

void TrendStats::reset() {
  EWAHBoolArray<uint32_t> *bitset = (EWAHBoolArray<uint32_t>*)_bitset;

  bitset->reset();
};

/* *************************************** */

void TrendStats::set(time_t when) {
  EWAHBoolArray<uint32_t> *bitset = (EWAHBoolArray<uint32_t>*)_bitset;

  bitset->set(when /* - (when % TIME_GRANULARITY) */);
};

/* *************************************** */

void TrendStats::setDump(stringstream* dump) {
  EWAHBoolArray<uint32_t> *bitset = (EWAHBoolArray<uint32_t>*)_bitset;

  bitset->read(*dump);
}

/* *************************************** */

void TrendStats::print() {

}
