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

char *strip_address(char *key)
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

int main(int argc, char *argv[])
{
  int i, c;
  redisContext *context;
  redisReply *offenders, *blacklist, *score;
  char *host = "localhost";
  int port = 6379;
  int threshold = 200;

  while((c = getopt (argc, argv, "h:p:t:")) != -1)
    switch(c)
      {
      case 'h':
        host = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 't':
        threshold = atoi(optarg);
        break;
      case '?':
        return 1;
      default:
        abort();
      }

  context = redisConnect(host, port);
  if (context == NULL || context->err) {
    if (context) {
      printf("Redis Connection Error: %s\n", context->errstr);
      redisFree(context);
    } else {
      perror("Connection Error: can't allocate redis context\n");
      redisFree(context);
    }
    return -1;
  }

  int printed = 0;
  char *address = malloc(16);
  offenders = redisCommand(context, "KEYS *:*:count");

  for (i = 0; i < offenders->elements; i++) {
    address = strip_address(offenders->element[i]->str);

    blacklist = redisCommand(context, "GET %s:repsheet:blacklist", address);
    if (blacklist && (blacklist->type != REDIS_REPLY_NIL) && (strcmp(blacklist->str, "true") == 0)) {
      freeReplyObject(blacklist);
      continue;
    }

    score = redisCommand(context, "GET %s", offenders->element[i]->str);
    if (score->type != REDIS_REPLY_NIL) {
      freeReplyObject(redisCommand(context, "HINCRBY backend:sweeper %s %s", address, score->str));
    }
    freeReplyObject(score);
    freeReplyObject(blacklist);
  }

  offenders = redisCommand(context, "HGETALL backend:sweeper");
  if ((offenders->type = REDIS_REPLY_ARRAY) && (offenders->elements > 0)) {
    for(i = 0; i < offenders->elements; i+=2) {
      if (atoi(offenders->element[i + 1]->str) > threshold) {
        if (!printed) {
          printf("Blacklisting the following repeat offenders (threshold == %d)\n", threshold);
          printed = 1;
        }

        redisCommand(context, "SET %s:repsheet:blacklist true", offenders->element[i]->str);
        printf("  %s (%s offenses)\n", offenders->element[i]->str, offenders->element[i + 1]->str);
      }
    }
  }

  freeReplyObject(offenders);
  freeReplyObject(redisCommand(context, "DEL backend:sweeper"));
  redisFree(context);

  return 0;
}
