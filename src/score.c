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

#include "score.h"

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

void score(redisContext *context)
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

      score = redisCommand(context, "GET %s", suspects->element[i]->str);
      if (score && score->type == REDIS_REPLY_STRING) {
        freeReplyObject(redisCommand(context, "ZINCRBY offenders %s %s", score->str, address));
	freeReplyObject(score);
      }
    }
    freeReplyObject(suspects);
  }

  free(address);
}
