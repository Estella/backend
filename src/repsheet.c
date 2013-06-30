/*
  Copyright 2013 Aaron Bedra

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "hiredis/hiredis.h"

#include "repsheet.h"

config_t config;

static redisContext *get_redis_context()
{
  redisContext *context;

  context = redisConnect(config.host, config.port);
  if (context == NULL || context->err) {
    if (context) {
      printf("Redis Connection Error: %s\n", context->errstr);
      redisFree(context);
    } else {
      perror("Connection Error: can't allocate redis context\n");
    }
    return NULL;
  } else {
    return context;
  }
}

static char *strip_address(char *key)
{
  char *ptr = strchr(key, ':');
  if (ptr == NULL) {
    return NULL;
  }

  int position = ptr - key;
  char* address = (char*) malloc((position + 1) * sizeof(char));

  memcpy(address, key, position);
  address[position] = '\0';

  return address;
}

static void score(redisContext *context)
{
  int i;
  redisReply *blacklist, *score, *suspects;
  char *address = malloc(16);

  freeReplyObject(redisCommand(context, "DEL offenders"));

  suspects = redisCommand(context, "KEYS *:*:count");
  if (suspects && suspects->type == REDIS_REPLY_ARRAY) {
    for (i = 0; i < suspects->elements; i++) {
      address = strip_address(suspects->element[i]->str);

      blacklist = redisCommand(context, "GET %s:repsheet:blacklist", address);
      if (blacklist && (blacklist->type == REDIS_REPLY_STRING) && (strcmp(blacklist->str, "true") == 0)) {
        freeReplyObject(blacklist);
        continue;
      }
      freeReplyObject(blacklist);

      score = redisCommand(context, "GET %s", suspects->element[i]->str);
      if (score && score->type == REDIS_REPLY_STRING) {
        freeReplyObject(redisCommand(context, "ZINCRBY offenders %s %s", score->str, address));
      }
      freeReplyObject(score);
    }
    freeReplyObject(suspects);
  }

  free(address);
}

static void blacklist_offenders(redisContext *context)
{
  int i;
  int printed = 0;
  redisReply *offenders, *whitelist;

  offenders = redisCommand(context, "ZRANGEBYSCORE offenders %d +inf", config.threshold);
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for(i = 0; i < offenders->elements; i++) {

      whitelist = redisCommand(context, "GET %s:repsheet:whitelist", offenders->element[i]->str);
      if (whitelist && (whitelist->type == REDIS_REPLY_STRING) && strcmp(whitelist->str, "true") == 0) {
        freeReplyObject(whitelist);
        continue;
      }

      if (!printed) {
        printf("Blacklisting the following repeat offenders (threshold == %d)\n", config.threshold);
        printed = 1;
      }

      freeReplyObject(redisCommand(context, "SET %s:repsheet:blacklist true", offenders->element[i]->str));
      freeReplyObject(redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", offenders->element[i]->str, config.expiry));
      printf("  %s\n", offenders->element[i]->str);
    }
    freeReplyObject(offenders);
  }
}

static void report(redisContext *context)
{
  int i;
  redisReply *top_ten, *score;

  top_ten = redisCommand(context, "ZREVRANGEBYSCORE offenders +inf 0");
  if (top_ten && (top_ten->type == REDIS_REPLY_ARRAY) && (top_ten->elements > 0)) {
    if (top_ten->elements > 10) {
      printf("Top 10 Suspsects (not yet blacklisted)\n");
      for(i = 0; i < 10; i++) {
        score = redisCommand(context, "ZSCORE offenders %s", top_ten->element[i]->str);
        if (score && score->type == REDIS_REPLY_STRING) {
          printf("  %s\t%s offenses\n", top_ten->element[i]->str, score->str);
          freeReplyObject(score);
        }
      }
    } else {
      printf("Top %zu Suspsects (not yet blacklisted)\n", top_ten->elements);
      for(i = 0; i < top_ten->elements; i++) {
        score = redisCommand(context, "ZSCORE offenders %s", top_ten->element[i]->str);
        if (score && score->type == REDIS_REPLY_STRING) {
          printf("  %s\t%s offenses\n", top_ten->element[i]->str, score->str);
          freeReplyObject(score);
        }
      }
    }
    freeReplyObject(top_ten);
  }
}

static void print_usage()
{
  printf("Repsheet Backend Version %s\n", VERSION);
  printf("usage: repsheet [-h] [-p] [-sbr]\n  -h <redis host>\n  -p <redis port>\n  -s (score actors)\n  -r (report top 10 offenders)\n  -b (blacklist offenders)\n");
}

int main(int argc, char *argv[])
{
  int c;
  redisContext *context;

  config.host = "localhost";
  config.port = 6379;
  config.threshold = 200;
  config.score = 0;
  config.report = 0;
  config.blacklist = 0;
  config.expiry = (24 * 60 * 60);

  while((c = getopt (argc, argv, "h:p:t:srbv")) != -1)
    switch(c)
      {
      case 'h':
        config.host = optarg;
        break;
      case 'p':
        config.port = atoi(optarg);
        break;
      case 't':
        config.threshold = atoi(optarg);
        break;
      case 's':
        config.score = 1;
        break;
      case 'r':
        config.report = 1;
        break;
      case 'b':
        config.blacklist = 1;
        break;
      case 'v':
        print_usage();
        return 0;
        break;
      case '?':
        return 1;
      default:
        print_usage();
        abort();
      }

  context = get_redis_context();
  if (context == NULL) {
    return -1;
  }

  if (!config.report && !config.blacklist && !config.score) {
    printf("No options specified, performing score operation.\nTo remove this message, specify -s (score) or [-r | -b] (report or blacklist)\n");
    score(context);
  }

  if (config.score) {
    score(context);
  }

  if (config.blacklist) {
    score(context);
    blacklist_offenders(context);
    score(context);
  }

  if (config.report) {
    score(context);
    report(context);
  }

  redisFree(context);

  return 0;
}
