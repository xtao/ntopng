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
  this->ifid = ifid;
  snprintf(filePath, sizeof(filePath), "%s/%d/top_talkers/",
	   ntop->get_working_dir(), ifid);
  strncpy(fileName, filename, sizeof(fileName));
  snprintf(fileFullPath, sizeof(fileFullPath), "%s/%d/top_talkers/%s",
	   ntop->get_working_dir(), ifid, filename);
  ntop->fixPath(filePath);
  ntop->fixPath(fileFullPath);

  if (sqlite3_open(fileFullPath, &db)) {
    printf("Unable to open %s: %s\n", fileFullPath, sqlite3_errmsg(db));
    ntop->getTrace()->traceEvent(TRACE_INFO, "Unable to open %s: %s",
                                fileFullPath, sqlite3_errmsg(db));
  }
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
 * @brief Database interface to add a new stats sampling
 * @details This function implements the database-specific layer for
 *          the historical database (as of now using SQLite3).
 *
 * @param timeinfo Localtime representation of the sampling point.
 * @param sampling String to be written at specified sampling point.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::insertSamplingDb(tm *timeinfo, char *sampling) {
  char key[MAX_KEY], query[MAX_QUERY];
  char *table_query;
  sqlite3_stmt *stmt;
  int rc = 0;

  if(!Utils::mkdir_tree(filePath)) {
    ntop->getTrace()->traceEvent(TRACE_WARNING,
				 "Unable to create directory %s", filePath);
    return -1;
  }

  strftime(key, sizeof(key), "%Y%m%d%H%M", timeinfo);

  table_query = strndup("CREATE TABLE IF NOT EXISTS MINUTE_STATS ("  \
                "TSTAMP VARCHAR PRIMARY KEY NOT NULL," \
                "STATS  TEXT NOT NULL);", MAX_QUERY);

  strncpy(query, "INSERT INTO MINUTE_STATS (TSTAMP, STATS) VALUES(?,?)",
          sizeof(query));

  m.lock(__FILE__, __LINE__);

  if (exec_query(table_query, NULL, NULL)) {
    rc = 1;
    goto out_unlock;
  }

  if (sqlite3_prepare(db, query, -1, &stmt, 0) ||
      sqlite3_bind_text(stmt, 1, key, strlen(key), SQLITE_TRANSIENT) ||
      sqlite3_bind_text(stmt, 2, sampling, strlen(sampling), SQLITE_STATIC)) {
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

  free(table_query);

  return rc;
}

/**
 * @brief Filesystem interface to add a new stats sampling
 * @details This function implements the filesystem-specific layer for
 *          the historical database.
 *
 * @param timeinfo Localtime representation of the sampling point.
 * @param sampling String to be written at specified sampling point.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::insertSamplingFs(tm *timeinfo, char *sampling) {
  char innerpath[MAX_PATH], buffer[MAX_PATH], filename[10];
  u_int len;

  strftime(innerpath, sizeof(innerpath), "%Y/%m/%d/%H", timeinfo);
  strftime(filename, sizeof(filename), "%M.json", timeinfo);

  snprintf(buffer, sizeof(buffer), "%s%s", filePath, innerpath);

  if(!Utils::mkdir_tree(buffer)) {
    ntop->getTrace()->traceEvent(TRACE_WARNING,
				 "Unable to create directory %s", buffer);
    return(-1);
  } else
    len = strlen(buffer);

  if(len < sizeof(buffer)) {
    FILE *fd;

    snprintf(&buffer[len], sizeof(buffer)-len, "%c%s", CONST_PATH_SEP, filename);

    if((fd = fopen(buffer, "w")) == NULL)
      ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to create file %s", buffer);
    else {
      fwrite(sampling, strlen(sampling), 1, fd);
      fclose(fd);
    }
    return(0);
  } else {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Buffer too short");
    return(-1);
  }
}

/**
 * @brief Interface function for insertion of a new sampling
 * @details This public method implements insertion of a new sampling,
 *          hiding the actual backend used to store it.
 *
 * @param timeinfo The sampling point expressed in localtime format.
 * @param sampling Pointer to a string keeping the sampling.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::insertSampling(tm *timeinfo, char *sampling) {
  if (!timeinfo || !sampling)
    return -1;
  return insertSamplingDb(timeinfo, sampling);
}

/* *************************************************************** */

/**
 * @brief Callback for completion of minute stata retrieval.
 *
 * @param data Pointer to exchange data used by SQLite and the callback.
 * @param argc Number of retrieved rows.
 * @param argv Content of retrieved rows.
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
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::getSamplingDb(time_t epoch, string *sampling) {

  char key[MAX_KEY], query[MAX_QUERY];
  struct stat buf;
  int rc;

  *sampling = "[ ]";

  strftime(key, sizeof(key), "%Y%m%d%H%M", localtime(&epoch));

  if (stat(fileFullPath, &buf))
    return 0;

  snprintf(query, sizeof(query), "SELECT STATS FROM MINUTE_STATS WHERE TSTAMP = %s",
           key);

  m.lock(__FILE__, __LINE__);

  rc = exec_query(query, get_sampling_db_callback, (void *)sampling);

  m.unlock(__FILE__, __LINE__);

  return rc;
}

/**
 * @brief Filesystem interface to retrieve a stats sampling
 * @details This function implements the filesystem-specific layer for
 *          the historical database.
 *
 * @param epoch Sampling point expressed in number of seconds from epoch.
 * @param sampling Pointer to a string to be filled with retrieved data.
 *
 * @return Zero in case of success, nonzero in case of error.
 */
int StatsManager::getSamplingFs(time_t epoch, string *sampling) {
  char innerpath[MAX_PATH], buffer[MAX_PATH];
  ifstream in;
  stringstream sstream;

  *sampling = "[ ]";

  strftime(innerpath, MAX_PATH, "%Y/%m/%d/%H/%M.json", localtime(&epoch));

  snprintf(buffer, sizeof(buffer), "%s%s", filePath, innerpath);

  in.open(buffer);
  if(in.fail())
    return 0;

  sstream << in.rdbuf();
  *sampling = sstream.str();
  in.close();

  return 0;
}

/**
 * @brief Interface function for retrieval of a sampling
 * @details This public method implements retrieval of an existing
 *          sampling, hiding the actual backend used to store it.
 *
 * @param epoch The sampling point expressed as number of seconds
 *              from epoch.
 * @param sampling Pointer to a string to be filled with the sampling.
 *
 * @return Zero in case of success, nonzero in case of failure.
 */
int StatsManager::getSampling(time_t epoch, string *sampling) {
  if (!sampling)
    return -1;
  return getSamplingDb(epoch, sampling);
}
