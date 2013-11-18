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

//TODO: write some tests for this
static int total_offenses(redisContext *context, char *actor)
{
  int i;
  int total = 0;
  redisReply *offenses;

  offenses = redisCommand(context, "ZRANGE %s:detected 0 -1 WITHSCORES", actor);
  if (offenses && offenses->type == REDIS_REPLY_ARRAY) {
    for(i = 1; i < offenses->elements; i += 2) {
      total += strtol(offenses->element[i]->str, 0, 10);
    }
    freeReplyObject(offenses);
  }

  return total;
}

void score(redisContext *context)
{
  int i, score;
  redisReply *suspects;

  redisCommand(context, "DEL offenders");

  suspects = redisCommand(context, "KEYS *:detected");
  if (suspects && suspects->type == REDIS_REPLY_ARRAY) {
    for (i = 0; i < suspects->elements; i++) {
      if (no_action_required(context, strip_address(suspects->element[i]->str))) {
        continue;
      }

      score = total_offenses(context, strip_address(suspects->element[i]->str));
      redisCommand(context, "ZINCRBY offenders %d %s", score, strip_address(suspects->element[i]->str));
    }
    freeReplyObject(suspects);
  }
}
