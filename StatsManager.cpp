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
 * @param num_days Number of days to use to purge statistics.
 * @param cache_name Name of the cache to purge statistics from.
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::deleteStatsOlderThan(unsigned num_days, const char *cache_name) {
  unsigned years, months, days;
  char key[MAX_KEY], query[MAX_QUERY];
  time_t rawtime;
  tm *timeinfo;
  int rc;

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

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

  snprintf(query, sizeof(query), "DELETE FROM %s WHERE "
                                 "CAST(TSTAMP AS INTEGER) < %s",
                                 cache_name, key);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, NULL, NULL);

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Database interface to add a new stats sampling
 * @details This function implements the database-specific layer for
 *          the historical database (as of now using SQLite3).
 *
 * @param timeinfo Localtime representation of the sampling point.
 * @param sampling String to be written at specified sampling point.
 * @param cache_name Name of the table to write the entry to.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::insertSamplingDb(tm *timeinfo, char *sampling,
                                   const char *cache_name) {
  char key[MAX_KEY], query[MAX_QUERY];
  sqlite3_stmt *stmt;
  int rc = 0;

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  strftime(key, sizeof(key), "%Y%m%d%H%M", timeinfo);

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
 * @brief Interface function for insertion of a new sampling
 * @details This public method implements insertion of a new sampling,
 *          hiding the actual backend used to store it.
 *
 * @param timeinfo The sampling point expressed in localtime format.
 * @param sampling Pointer to a string keeping the sampling.
 * @param cache_name Name of the cache to insert the sampling in.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::insertSampling(tm *timeinfo, char *sampling,
                                 const char *cache_name) {
  if (!timeinfo || !sampling || !cache_name)
    return -1;
  return insertSamplingDb(timeinfo, sampling, cache_name);
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
 * @param epoch Sampling point expressed in number of seconds from epoch.
 * @param sampling Pointer to a string to be filled with retrieved data.
 * @param cache_name Name of the cache to retrieve stats from.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::getSamplingDb(time_t epoch, string *sampling,
                                const char *cache_name) {

  char key[MAX_KEY], query[MAX_QUERY];
  int rc;

  *sampling = "[ ]";

  if (!db)
    return -1;

  if (openCache(cache_name))
    return -1;

  strftime(key, sizeof(key), "%Y%m%d%H%M", localtime(&epoch));

  snprintf(query, sizeof(query), "SELECT STATS FROM %s WHERE TSTAMP = %s",
           cache_name, key);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, get_sampling_db_callback, (void *)sampling);

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Interface function for retrieval of a sampling
 * @details This public method implements retrieval of an existing
 *          sampling, hiding the actual backend used to store it.
 *
 * @param epoch The sampling point expressed as number of seconds
 *              from epoch.
 * @param sampling Pointer to a string to be filled with the sampling.
 * @param cache_name Name of the cache to get stats from.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::getSampling(time_t epoch, string *sampling,
                              const char *cache_name) {
  if (!sampling || !cache_name)
    return -1;
  return getSamplingDb(epoch, sampling, cache_name);
}
