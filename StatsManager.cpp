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

StatsManager::StatsManager(int ifid, const char *filename) {
  char filePath[MAX_PATH], fileFullPath[MAX_PATH], fileName[MAX_PATH];

  this->ifid = ifid;
  MINUTE_CACHE_NAME = "MINUTE_STATS";
  HOUR_CACHE_NAME = "HOUR_STATS";

  snprintf(filePath, sizeof(filePath), "%s/%d/top_talkers/",
           ntop->get_working_dir(), ifid);
  strncpy(fileName, filename, sizeof(fileName));
  snprintf(fileFullPath, sizeof(fileFullPath), "%s/%d/top_talkers/%s",
	   ntop->get_working_dir(), ifid, filename);
  ntop->fixPath(filePath);
  ntop->fixPath(fileFullPath);

  if(!Utils::mkdir_tree(filePath)) {
    ntop->getTrace()->traceEvent(TRACE_WARNING,
				 "Unable to create directory %s", filePath);
    return;
  }
  if (sqlite3_open(fileFullPath, &db))
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to open %s: %s",
                                fileFullPath, sqlite3_errmsg(db));
}

StatsManager::~StatsManager() {
  sqlite3_close(db);
}

/**
 * @brief Executes a database query on an already opened SQLite3 DB
 * @brief This function implements handling of a direct query on
 *        a SQLite3 database, hiding DB-specific syntax and error
 *        handling.
 *
 * @param db_query A string keeping the query to be executed.
 * @param callback Callback to be executed by the DB in case the query
 *                 execution is successful.
 * @param payload A pointer to be passed to the callback in case it
 *                is actually executed.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::exec_query(char *db_query,
                             int (*callback)(void *, int, char **, char **),
                             void *payload) {
  char *zErrMsg = 0;

  if(sqlite3_exec(db, db_query, callback, payload, &zErrMsg)) {
    printf("SQL error: %s\n", sqlite3_errmsg(db));
    ntop->getTrace()->traceEvent(TRACE_INFO, "SQL Error: %s", zErrMsg);
    sqlite3_free(zErrMsg);
    return 1;
  }

  return 0;
}

/**
 * @brief Opens a new cache to be used to store statistics.
 * @brief This function implements opening a new cache to store stats
 *        in; it is as of now based on SQLite3, and equals a new cache
 *        to a new table.
 *
 * @param cache_name Name of the cache to be opened.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::openCache(const char *cache_name)
{
  char table_query[MAX_QUERY];
  int rc;

  if (!db)
    return 1;

  if (caches[cache_name])
    return 0;

  snprintf(table_query, sizeof(table_query),
                       "CREATE TABLE IF NOT EXISTS %s "
                       "(TSTAMP VARCHAR PRIMARY KEY NOT NULL,"
                       "STATS TEXT NOT NULL);", cache_name);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(table_query, NULL, NULL);

  if (!rc)
    caches[cache_name] = true;

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Database interface to implement stats purging.
 * @details This function implements the database-specific code
 *          to delete stats older than a certain number of days.
 *
 * @todo Compute years better.
 *
 * @param cache_name Name of the cache to purge statistics from.
 * @param key Key to use as boundary.
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::deleteStatsOlderThan(const char *cache_name,
				       const char *key) {
  char query[MAX_QUERY];
  int rc;

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  snprintf(query, sizeof(query), "DELETE FROM %s WHERE "
                                 "CAST(TSTAMP AS INTEGER) < %s",
                                 cache_name, key);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, NULL, NULL);

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Minute stats interface to database purging.
 * @details This function hides cache-specific details (e.g. building the key
 *          for the minute stats cache.
 *
 * @param num_days Number of days to use to purge statistics.
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::deleteMinuteStatsOlderThan(unsigned num_days) {
  unsigned years, months, days;
  char key[MAX_KEY];
  time_t rawtime;
  tm *timeinfo;

  months = num_days / 30;
  days = num_days % 30;
  years = months / 12;
  months = months % 12;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  timeinfo->tm_year -= years;
  timeinfo->tm_mon -= months;
  timeinfo->tm_mday -= days;
  strftime(key, sizeof(key), "%Y%m%d%H%M", timeinfo);

  return deleteStatsOlderThan(MINUTE_CACHE_NAME, key);
}

struct statsManagerRetrieval {
  vector<string> rows;
  int num_vals;
};

/**
 * @brief Callback for completion of retrieval of an interval of stats
 *
 * @param data Pointer to exchange data used by SQLite and the callback.
 * @param argc Number of retrieved columns.
 * @param argv Content of retrieved columns.
 * @param azColName Retrieved columns name.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
static int get_samplings_db(void *data, int argc,
                            char **argv, char **azColName) {
  struct statsManagerRetrieval *retr = (struct statsManagerRetrieval *)data;

  if (argc > 1) return -1;

  retr->rows.push_back(argv[0]);
  retr->num_vals++;

  return 0;
}

/**
 * @brief Retrieve an interval of samplings from a database
 * @details This function implements the database-specific code
 *          to retrieve an interval of samplings.
 *
 * @param vals Pointer to a string array that will keep the result.
 * @param num_vals Pointer to an integer that will keep the number
 *        of retrieved sampling points.
 * @param cache_name Pointer to the name of the cache to retrieve
 *        stats from.
 * @param key_start Key to use as left boundary.
 * @param key_end Key to use as right boundary.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::retrieveStatsInterval(char ***vals,
                                        int *num_vals,
					const char *cache_name,
                                        const char *key_start,
                                        const char *key_end) {
  char query[MAX_QUERY];
  struct statsManagerRetrieval retvals;
  vector<string> rows;
  int rc;

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  memset(&retvals, 0, sizeof(retvals));

  snprintf(query, sizeof(query), "SELECT STATS FROM %s WHERE TSTAMP >= %s "
				 "AND TSTAMP <= %s",
           cache_name, key_start, key_end);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, get_samplings_db, (void *)&retvals);

  m.unlock(__FILE__, __LINE__);

  if (retvals.num_vals > 0)
    *vals = (char**) malloc(retvals.num_vals * sizeof(char*));
  for (unsigned i = 0 ; i < retvals.rows.size() ; i++)
    (*vals)[i] = strndup(retvals.rows[i].c_str(), MAX_QUERY);

  *num_vals = retvals.num_vals;

  return rc;
}

/**
 * @brief Retrieve an interval of samplings from the minute stats cache
 * @details This function implements the database-specific code
 *          to retrieve an interval of samplings masking out cache-specific
 *          details concerning the minute stats cache.
 *
 * @param epoch_start Left boundary of the interval.
 * @param epoch_end Right boundary of the interval.
 * @param vals Pointer to a string array that will keep the result.
 * @param num_vals Pointer to an integer that will keep the number
 *        of retrieved sampling points.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::retrieveMinuteStatsInterval(time_t epoch_start,
				              time_t epoch_end,
					      char ***vals,
                                              int *num_vals) {
  char key_start[MAX_KEY], key_end[MAX_KEY];

  strftime(key_start, sizeof(key_start), "%Y%m%d%H%M", localtime(&epoch_start));
  strftime(key_end, sizeof(key_end), "%Y%m%d%H%M", localtime(&epoch_end));

  return retrieveStatsInterval(vals, num_vals, MINUTE_CACHE_NAME, key_start, key_end);
}

/**
 * @brief Database interface to add a new stats sampling
 * @details This function implements the database-specific layer for
 *          the historical database (as of now using SQLite3).
 *
 * @param sampling String to be written at specified sampling point.
 * @param cache_name Name of the table to write the entry to.
 * @param key Key to use to insert the sampling.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::insertSampling(char *sampling, const char *cache_name,
                                 const char *key) {
  char query[MAX_QUERY];
  sqlite3_stmt *stmt;
  int rc = 0;

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  snprintf(query, sizeof(query), "INSERT INTO %s (TSTAMP, STATS) VALUES(?,?)",
                                 cache_name);

  m.lock(__FILE__, __LINE__);

  if (sqlite3_prepare(db, query, -1, &stmt, 0) ||
      sqlite3_bind_text(stmt, 1, key, strlen(key), SQLITE_TRANSIENT) ||
      sqlite3_bind_text(stmt, 2, sampling, strlen(sampling), SQLITE_TRANSIENT)) {
    rc = 1;
    goto out_unlock;
  }

  while((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
    if (rc == SQLITE_ERROR) {
        ntop->getTrace()->traceEvent(TRACE_INFO, "SQL Error: step");
        rc = 1;
        goto out;
    }
  }

  rc = 0;
out:
  sqlite3_finalize(stmt);

out_unlock:
  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Interface function for insertion of a new minute stats sampling
 * @details This public method implements insertion of a new sampling,
 *          hiding cache-specific details related to minute stats.
 *
 * @param timeinfo The sampling point expressed in localtime format.
 * @param sampling Pointer to a string keeping the sampling.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::insertMinuteSampling(tm *timeinfo, char *sampling) {
  char key[MAX_KEY];

  if (!timeinfo || !sampling)
    return -1;

  strftime(key, sizeof(key), "%Y%m%d%H%M", timeinfo);

  return insertSampling(sampling, MINUTE_CACHE_NAME, key);
}

/**
 * @brief Interface function for insertion of a new hour stats sampling
 * @details This public method implements insertion of a new sampling,
 *          hiding cache-specific details related to hour stats.
 *
 * @param timeinfo The sampling point expressed in localtime format.
 * @param sampling Pointer to a string keeping the sampling.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::insertHourSampling(tm *timeinfo, char *sampling) {
  char key[MAX_KEY];

  if (!timeinfo || !sampling)
    return -1;

  strftime(key, sizeof(key), "%Y%m%d%H", timeinfo);

  return insertSampling(sampling, HOUR_CACHE_NAME, key);
}

/* *************************************************************** */

/**
 * @brief Callback for completion of minute stats retrieval.
 *
 * @param data Pointer to exchange data used by SQLite and the callback.
 * @param argc Number of retrieved columns.
 * @param argv Content of retrieved columns.
 * @param azColName Retrieved columns name.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
static int get_sampling_db_callback(void *data, int argc,
                                    char **argv, char **azColName) {
  string *buffer = (string *)data;

  if (argc > 1)
    ntop->getTrace()->traceEvent(TRACE_INFO, "%s: more than one row (%d) returned",
                                __FUNCTION__, argc);

  buffer->assign(argv[0]);

  return 0;
}

/**
 * @brief Database interface to retrieve a stats sampling
 * @details This function implements the database-specific layer for
 *          the historical database (as of now using SQLite3).
 *
 * @param sampling Pointer to a string to be filled with retrieved data.
 * @param cache_name Name of the cache to retrieve stats from.
 * @param key Key used to retrieve the sampling.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::getSampling(string *sampling, const char *cache_name,
                              const char *key) {

  char query[MAX_QUERY];
  int rc;

  *sampling = "[ ]";

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  snprintf(query, sizeof(query), "SELECT STATS FROM %s WHERE TSTAMP = %s",
           cache_name, key);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, get_sampling_db_callback, (void *)sampling);

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Interface function for retrieval of a minute stats sampling
 * @details This public method implements retrieval of an existing
 *          sampling, hiding cache-specific details related to minute stats.
 *
 * @param epoch The sampling point expressed as number of seconds
 *              from epoch.
 * @param sampling Pointer to a string to be filled with the sampling.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::getMinuteSampling(time_t epoch, string *sampling) {
  char key[MAX_KEY];

  if (!sampling)
    return -1;

  strftime(key, sizeof(key), "%Y%m%d%H%M", localtime(&epoch));

  return getSampling(sampling, MINUTE_CACHE_NAME, key);
}
