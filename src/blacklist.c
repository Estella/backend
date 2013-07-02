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

#include "blacklist.h"

void blacklist(redisContext *context, config_t config)
{
  int i;
  int printed = 0;
  redisReply *offenders, *whitelist;

  offenders = redisCommand(context, "ZRANGEBYSCORE offenders %d +inf", config.threshold);
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for(i = 0; i < offenders->elements; i++) {

      whitelist = redisCommand(context, "GET %s:repsheet:whitelist", offenders->element[i]->str);
      if (whitelist && whitelist->type == REDIS_REPLY_STRING && strcmp(whitelist->str, "true") == 0) {
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
