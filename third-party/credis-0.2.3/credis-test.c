/* credis-test.c -- a sample test application using credis (C client library 
 * for Redis)
 *
 * Copyright (c) 2009-2010, Jonas Romfelt <jonas at romfelt dot se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Credis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "credis.h"


long timer(int reset) 
{
  static long start=0; 
  struct timeval tv;

  gettimeofday(&tv, NULL);

  /* return timediff */
  if (!reset) {
    long stop = ((long)tv.tv_sec)*1000 + tv.tv_usec/1000;
    return (stop - start);
  }

  /* reset timer */
  start = ((long)tv.tv_sec)*1000 + tv.tv_usec/1000;

  return 0;
}

unsigned long getrandom(unsigned long max)
{
  return (1 + (unsigned long) ( ((double)max) * (rand() / (RAND_MAX + 1.0))));
}

void randomize()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

#define DUMMY_DATA "some dummy data string"
#define LONG_DATA 50000

int main(int argc, char **argv) {
  REDIS redis;
  REDIS_INFO info;
  char *val, **valv, lstr[50000];
  const char *keyv[] = {"kalle", "adam", "unknown", "bertil", "none"};
  int rc, keyc=5, i;
  double score1, score2;

  redis = credis_connect(NULL, 0, 10000);
  if (redis == NULL) {
    printf("Error connecting to Redis server. Please start server to run tests.\n");
    exit(1);
  }

  if (argc == 2) {
    int i;
    long t;
    int num = atoi(argv[1]);
    printf("Sending %d 'set' commands ...\n", num);
    timer(1);
    for (i=0; i<num; i++) {
      if (credis_set(redis, "kalle", "qwerty") != 0)
        printf("get returned error\n");
    }
    t = timer(0);
    printf("done! Took %.3f seconds, that is %ld commands/second\n", ((float)t)/1000, (num*1000)/t);
    exit(0);
  }

  printf("Testing a number of credis functions. To perform a simplistic set-command\n"\
         "benchmark, run: `%s <num>' where <num> is the number\n"\
         "of set-commands to send.\n\n", argv[0]);

  printf("\n\n************* misc info ************************************ \n");

  rc = credis_ping(redis);
  printf("ping returned: %d\n", rc);

  rc = credis_lastsave(redis);
  printf("lastsave returned: %d\n", rc);

  rc = credis_info(redis, &info);
  printf("info returned %d\n", rc);
  printf("> redis_version: %s\n", info.redis_version);
  printf("> arch_bits: %d\n", info.arch_bits);
  printf("> multiplexing_api: %s\n", info.multiplexing_api);
  printf("> process_id: %ld\n", info.process_id);
  printf("> uptime_in_seconds: %ld\n", info.uptime_in_seconds);
  printf("> uptime_in_days: %ld\n", info.uptime_in_days);
  printf("> connected_clients: %d\n", info.connected_clients);
  printf("> connected_slaves: %d\n", info.connected_slaves);
  printf("> blocked_clients: %d\n", info.blocked_clients);
  printf("> used_memory: %zu\n", info.used_memory);
  printf("> used_memory_human: %s\n", info.used_memory_human);
  printf("> changes_since_last_save: %lld\n", info.changes_since_last_save);
  printf("> bgsave_in_progress: %d\n", info.bgsave_in_progress);
  printf("> last_save_time: %ld\n", info.last_save_time);
  printf("> bgrewriteaof_in_progress: %d\n", info.bgrewriteaof_in_progress);
  printf("> total_connections_received: %lld\n", info.total_connections_received);
  printf("> total_commands_processed: %lld\n", info.total_commands_processed);
  printf("> expired_keys: %lld\n", info.expired_keys);
  printf("> hash_max_zipmap_entries: %zu\n", info.hash_max_zipmap_entries);
  printf("> hash_max_zipmap_value: %zu\n", info.hash_max_zipmap_value);
  printf("> pubsub_channels: %ld\n", info.pubsub_channels);
  printf("> pubsub_patterns: %u\n", info.pubsub_patterns);
  printf("> vm_enabled: %d\n", info.vm_enabled);
  printf("> role: %d\n", info.role);


  printf("\n\n************* get/set ************************************ \n");

  rc = credis_set(redis, "kalle", "kula");
  printf("set kalle=kula returned: %d\n", rc);

  rc = credis_get(redis, "kalle", &val);
  printf("get kalle returned: %s\n", val);

  rc = credis_type(redis, "someunknownkey");
  printf("get type unknown key returned: %d\n", rc);

  rc = credis_type(redis, "kalle");
  printf("get type known key returned: %d\n", rc);

  rc = credis_getset(redis, "kalle", "buhu", &val);
  printf("getset kalle=buhu returned: %s\n", val);

  rc = credis_get(redis, "kalle", &val);
  printf("get kalle returned: %s\n", val);

  rc = credis_del(redis, "kalle");
  printf("del kalle returned: %d\n", rc);

  rc = credis_get(redis, "kalle", &val);
  printf("get kalle returned: %s\n", val);

  rc = credis_set(redis, "adam", "aaa");
  rc = credis_set(redis, "bertil", "bbbbbbb");
  rc = credis_set(redis, "caesar", "cccc");
  rc = credis_get(redis, "adam", &val);
  printf("get adam returned: %s\n", val);
  rc = credis_get(redis, "bertil", &val);
  printf("get bertil returned: %s\n", val);
  rc = credis_get(redis, "caesar", &val);
  printf("get caesar returned: %s\n", val);

  rc = credis_mget(redis, keyc, keyv, &valv);
  printf("mget returned: %d\n", rc);
  for (i = 0; i < rc; i++)
    printf(" % 2d: %s\n", i, valv[i]);

  rc = credis_keys(redis, "*", &valv);
  printf("keys returned: %d\n", rc);
  for (i = 0; i < rc; i++)
    printf(" % 2d: %s\n", i, valv[i]);


  printf("\n\n************* sets ************************************ \n");

  rc = credis_sadd(redis, "fruits", "banana");
  printf("sadd returned: %d\n", rc);

  rc = credis_sismember(redis, "fruits", "banana");
  printf("sismember returned: %d\n", rc);

  rc = credis_sadd(redis, "fruits", "apple");
  printf("sadd returned: %d\n", rc);

  rc = credis_srem(redis, "fruits", "banana");
  printf("srem returned: %d\n", rc);

  rc = credis_sismember(redis, "fruits", "banana");
  printf("sismember returned: %d\n", rc);

  rc = credis_srem(redis, "fruits", "orange");
  printf("srem returned: %d\n", rc);


  printf("\n\n************* lists ************************************ \n");

  rc = credis_llen(redis, "mylist");
  printf("length of list: %d\n", rc);

  rc = credis_del(redis, "mylist");
  printf("del returned: %d\n", rc);

  rc = credis_llen(redis, "mylist");
  printf("length of list: %d\n", rc);

  rc = credis_rpush(redis, "kalle", "first");
  printf("rpush returned: %d\n", rc);

  rc = credis_rpush(redis, "mylist", "first");
  printf("rpush returned: %d\n", rc);

  rc = credis_rpush(redis, "mylist", "right");
  printf("rpush returned: %d\n", rc);

  rc = credis_lpush(redis, "mylist", "left");
  printf("lpush returned: %d\n", rc);

  rc = credis_lrange(redis, "mylist", 0, 2, &valv);
  printf("lrange (0, 2) returned: %d\n", rc);
  for (i = 0; i < rc; i++)
    printf(" % 2d: %s\n", i, valv[i]);

  rc = credis_lrange(redis, "mylist", 0, -1, &valv);
  printf("lrange (0, -1) returned: %d\n", rc);
  for (i = 0; i < rc; i++)
    printf(" % 2d: %s\n", i, valv[i]);

  /* generate some test data */
  randomize();
  for (i = 0; i < LONG_DATA; i++)
    lstr[i] = ' ' + getrandom('~' - ' ');
  lstr[i-1] = 0;
  rc = credis_lpush(redis, "mylist", lstr);
  printf("rpush returned: %d\n", rc);

  rc = credis_lrange(redis, "mylist", 0, 0, &valv);
  if(valv && valv[0] != NULL)
    printf("lrange (0, 0) returned: %d, strncmp() returend %d\n", rc, strncmp(valv[0], lstr, LONG_DATA-1));

  rc = credis_llen(redis, "mylist");
  printf("length of list: %d\n", rc);

  rc = credis_lrange(redis, "not_exists", 0, -1, &valv);
  printf("lrange (0, -1) returned: %d\n", rc);
  for (i = 0; i < rc; i++)
    printf(" % 2d: %s\n", i, valv[i]);

  rc = credis_del(redis, "mylist");
  printf("del returned: %d\n", rc);

  rc = credis_llen(redis, "mylist");
  printf("length of list: %d\n", rc);

  printf("Adding 200 items to list\n");
  for (i = 0; i < 200; i++) {
    char str[100];
    sprintf(str, "%d%s%d", i, DUMMY_DATA, i);
    rc = credis_rpush(redis, "mylist", str);
    if (rc != 0)
      printf("rpush returned: %d\n", rc);
  }

  rc = credis_lrange(redis, "mylist", 0, 200, &valv);
  printf("lrange (0, 200) returned: %d, verifying data ... ", rc);
  for (i = 0; i < rc; i++) {
    char str[100];
    sprintf(str, "%d%s%d", i, DUMMY_DATA, i);
    if (strncmp(valv[i], str, strlen(str)))
      printf("\nreturned item (%d) data differs: '%s' != '%s'", i, valv[i], str);
  }  
  printf("all data verified!\n");

  printf("Testing lpush and lrem\n");
  rc = credis_lpush(redis, "cars", "volvo");
  rc = credis_lpush(redis, "cars", "saab");
  rc = credis_lrange(redis, "cars", 0, 200, &valv);
  printf("lrange (0, 200) returned: %d items\n", rc);
  for (i = 0; i < rc; i++) 
      printf("  %02d: %s\n", i, valv[i]);
  rc = credis_lrem(redis, "cars", 1, "volvo");
  printf("credis_lrem() returned %d\n", rc);
  rc = credis_lrange(redis, "cars", 0, 200, &valv);
  printf("lrange (0, 200) returned: %d items\n", rc);
  for (i = 0; i < rc; i++) 
      printf("  %02d: %s\n", i, valv[i]);
  rc = credis_lrem(redis, "cars", 1, "volvo");

  printf("Testing lset\n");
  rc = credis_lset(redis, "cars", 2, "koenigsegg");
  printf("lrange (0, 200) returned: %d items\n", rc);
  for (i = 0; i < rc; i++) 
      printf("  %02d: %s\n", i, valv[i]);
  rc = credis_lrem(redis, "cars", 1, "volvo");



  printf("\n\n************* sorted sets ********************************** \n");

  score1 = 3.5;
  rc = credis_zincrby(redis, "zkey", score1, "member1", &score2);
  printf("zincrby returned: %d, score=%f, new_score=%f\n", rc, score1, score2);
  rc = credis_zincrby(redis, "zkey", score1, "member1", &score2);
  printf("zincrby returned: %d, score=%f, new_score=%f\n", rc, score1, score2);
  score2 = 123;
  rc = credis_zscore(redis, "zkey", "member1", &score2);
  printf("zscore returned: %d, score=%f\n", rc, score2);
  rc = credis_zscore(redis, "zkey_unknown", "member1", &score2);
  printf("zscore (unknown key) returned: %d, score=%f\n", rc, score2);
  rc = credis_zscore(redis, "zkey", "member_unknown", &score2);
  printf("zscore (unknown member) returned: %d, score=%f\n", rc, score2);

  rc = credis_zrank(redis, "zkey", "member1");
  printf("zrank returned: %d\n", rc);
  rc = credis_zrevrank(redis, "zkey", "member1");
  printf("zrevrank returned: %d\n", rc);
  if (rc < 0)
    printf("Error message: %s\n", credis_errorreply(redis));
 
  credis_close(redis);

  return 0;
}
