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

void Categorization::categorizeHostName(char *_url) {
  char url[256];
  struct http_response *hresp;

  snprintf(url, sizeof(url), "http://service.block.si/getRating?url=%s&apikey=%s", _url, license_key);

  hresp = http_get(url, "User-agent:ntopng\r\n");

  printf("%d\n", hresp->status_code_int);
  printf("%s\n", hresp->body);
  http_response_free(hresp);
}

/* **************************************************** */

static void* categorizeLoop(void* ptr) {
  Categorization *a = (Categorization*)ptr;
  Redis *r = ntop->getRedis();

  while(!ntop->getGlobals()->isShutdown()) {
    char domain_name[64];

    int rc = r->popDomainToCategorize(domain_name, sizeof(domain_name));

    if(rc == 0) {
      a->categorizeHostName(domain_name);
    } else
      sleep(1);
  }

  return(NULL);
}

/* **************************************************** */

void Categorization::startCategorizeCategorizationLoop() {
  pthread_create(&categorizeThreadLoop, NULL, categorizeLoop, (void*)this);
}

