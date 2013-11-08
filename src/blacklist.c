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

static int no_action_required(redisContext *context, char *actor)
{
  redisReply *noop;

  noop = redisCommand(context, "KEYS %s:repsheet:*", actor);
  if (noop && noop->elements > 0) {
    freeReplyObject(noop);
    return 1;
  }

  return 0;
}

static int threshold_exceeded(redisContext *context, int threshold, char *actor)
{
  redisReply *score;

  score = redisCommand(context, "ZSCORE offenders %s", actor);
  if (score && atoi(score->str) > threshold) {
    freeReplyObject(score);
    return 1;
  }

  return 0;
}

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

static void blacklist_and_expire(redisContext *context, int expiry, char *actor, char *message)
{
  redisReply *ttl;

  redisCommand(context, "SET %s:repsheet:blacklist true", actor);
  redisCommand(context, "SADD repsheet:blacklist:history %s", actor);

  ttl = redisCommand(context, "TTL %s:requests", actor);
  if (ttl && ttl->integer > 0) {
    redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", actor, ttl->integer);
    freeReplyObject(ttl);
  } else {
    redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", actor, expiry);
  }

  printf("Actor %s has been blacklisted: %s\n", actor, message);
}

void blacklist(redisContext *context, config_t config)
{
  int i;
  int printed = 0;
  redisReply *offenders;

  offenders = redisCommand(context, "ZRANGE offenders 0 -1");
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for(i = 0; i < offenders->elements; i++) {
      if (no_action_required(context, offenders->element[i]->str)) {
        continue;
      }

      if(historical_offender(context, offenders->element[i]->str)) {
	blacklist_and_expire(context, config.expiry, offenders->element[i]->str, HISTORY_MESSAGE);
      } else if (threshold_exceeded(context, config.threshold, offenders->element[i]->str)) {
        blacklist_and_expire(context, config.expiry, offenders->element[i]->str, THRESHOLD_MESSAGE);
      }
    }
    freeReplyObject(offenders);
  }
}
