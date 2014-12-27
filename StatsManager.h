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

#ifndef _STATS_MANAGER_H_
#define _STATS_MANAGER_H_

#include "ntop_includes.h"
#include <map>

class StatsManager {
public:
    StatsManager(int ifid, const char *dbname);
    ~StatsManager();
    int insertSampling(tm *timeinfo, char *sampling, const char *cache_name);
    int getSampling(time_t epoch, string *sampling, const char *cache_name);
    int openCache(const char *cache_name);
    int deleteStatsOlderThan(unsigned num_days, const char *cache_name);
private:
    static const int MAX_QUERY = 10000;
    static const int MAX_KEY = 20;
    int ifid;
    char filePath[MAX_PATH]; /* legacy FS interface only */
    /*
     * map has O(log(n)) access time, but we suppose the number
     * of caches is not huge
     */
    std::map<string, bool> caches;
    Mutex m;
    sqlite3 *db;
    int exec_query(char *db_query,
                   int (*callback)(void *, int, char **, char **),
                   void *payload);
    int insertSamplingDb(tm *timeinfo, char *sampling, const char *cache_name);
    int insertSamplingFs(tm *timeinfo, char *sampling);
    int getSamplingDb(time_t epoch, string *sampling, const char *cache_name);
    int getSamplingFs(time_t epoch, string *sampling);
};

#endif /* _STATS_MANAGER_H_ */
