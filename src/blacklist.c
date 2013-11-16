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
#include "blacklist.h"

static int historical_offender(redisContext *context, char *actor)
{
  redisReply *reply;

  reply = redisCommand(context, "SISMEMBER repsheet:blacklist:history %s", actor);
  if (reply) {
    if (reply->integer == 1) {
      freeReplyObject(reply);
      return 1;
    }
    freeReplyObject(reply);
  }

  return 0;
}

void analyze_offenders(redisContext *context, config_t config)
{
  int i, score;
  int printed = 0;
  redisReply *offenders;

  offenders = redisCommand(context, "ZRANGE offenders 0 -1 WITHSCORES");
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for(i = 0; i < offenders->elements; i += 2) {
      if (no_action_required(context, offenders->element[i]->str)) {
        continue;
      }

      if(historical_offender(context, offenders->element[i]->str)) {
        blacklist_and_expire(context, config.expiry, offenders->element[i]->str, HISTORY_MESSAGE, (int)NULL);
	continue;
      }

      score = strtol(offenders->element[i+1]->str, 0, 10);
      if (score >= config.threshold) {
        blacklist_and_expire(context, config.expiry, offenders->element[i]->str, THRESHOLD_MESSAGE, score);
      }
    }
    freeReplyObject(offenders);
  }
}
