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
#include "http-client-c.h"

/* **************************************** */

Categorization::Categorization(char *_license_key) {
  license_key = _license_key ? strdup(_license_key) : NULL;
  num_categorized_categorizationes = num_categorized_fails = 0;
}

/* ******************************************* */

char* Categorization::findCategory(char *url) {
  return(NULL);
}

/* **************************************** */

Categorization::~Categorization() {
  void *res;

  if(license_key != NULL) {
    pthread_join(categorizeThreadLoop, &res);
    
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Categorization resolution stats [%u categorized][%u failures]",
				 num_categorized_categorizationes, num_categorized_fails);
  }
}

/* ***************************************** */

void Categorization::categorizeHostName(char *_url, char *buf, u_int buf_len) {
  char key[256];

  snprintf(key, sizeof(key), "domain.categorized.%s", _url);
  if(ntop->getRedis()->get(key, buf, buf_len) == 0) {
    ntop->getRedis()->expire(key, 86400);
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s => %s (cached)", _url, buf);
  } else {
    struct http_response *hresp;
    char url_buf[256];

    snprintf(url_buf, sizeof(url_buf), "%s?url=%s&apikey=%s", CATEGORIZATION_URL, _url, license_key);

    hresp = http_get(url_buf, "User-agent:ntopng\r\n");

#if 0
    printf("%d\n", hresp->status_code_int);
    printf("%s\n", hresp->body);
#endif

    buf[0] = '\0';
    if((hresp->status_code_int == 200) || (hresp->status_code_int == 0)) {
      char *doublecolumn = strrchr(hresp->body, ':');

      if(doublecolumn) {
	char *end;

	doublecolumn++;
	doublecolumn++;

	if((end = strchr(doublecolumn, '"')) != NULL) {
	  end[0] = '\0';

	  ntop->getTrace()->traceEvent(TRACE_NORMAL, "%s => %s", _url, doublecolumn);
	  ntop->getRedis()->set(key, doublecolumn, 86400);
	  snprintf(buf, buf_len, "%s", doublecolumn);
	}
      }
    }

    http_response_free(hresp);
  }
}

/* **************************************************** */

static void* categorizeLoop(void* ptr) {
  Categorization *a = (Categorization*)ptr;
  Redis *r = ntop->getRedis();

  while(!ntop->getGlobals()->isShutdown()) {
    char domain_name[64];

    int rc = r->popDomainToCategorize(domain_name, sizeof(domain_name));

    if(rc == 0) {
      char buf[8];
      
      a->categorizeHostName(domain_name, buf, sizeof(buf));
    } else
      sleep(1);
  }

  return(NULL);
}

/* **************************************************** */

void Categorization::startCategorizeCategorizationLoop() {
  pthread_create(&categorizeThreadLoop, NULL, categorizeLoop, (void*)this);
}

