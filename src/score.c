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

#include "util.h"
#include "score.h"

void score(redisContext *context)
{
  int i, j;
  int total = 0;
  redisReply *ignore, *scores, *suspects;
  char *address = malloc(16);

  redisCommand(context, "DEL offenders");

  suspects = redisCommand(context, "KEYS *:detected");
  if (suspects && suspects->type == REDIS_REPLY_ARRAY) {
    for (i = 0; i < suspects->elements; i++) {
      address = strip_address(suspects->element[i]->str);

      ignore = redisCommand(context, "KEYS %s:repsheet:*", address);
      if (ignore && (ignore->type == REDIS_REPLY_ARRAY) && (ignore->elements > 0)) {
        freeReplyObject(ignore);
        continue;
      }

      scores = redisCommand(context, "ZRANGE %s 0 -1 WITHSCORES", suspects->element[i]->str);
      if (scores && scores->type == REDIS_REPLY_ARRAY) {
        for(j = 1; j < scores->elements; j+=2) {
          total += atoi(scores->element[j]->str);
        }
        freeReplyObject(scores);
        redisCommand(context, "ZINCRBY offenders %d %s", total, address);
        total = 0;
      }
    }
    freeReplyObject(suspects);
    free(address);
  }
}
