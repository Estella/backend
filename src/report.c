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

#include "report.h"

void report(redisContext *context, config_t config)
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
