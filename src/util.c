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

int no_action_required(redisContext *context, char *actor)
{
  redisReply *noop;

  noop = redisCommand(context, "KEYS %s:repsheet:*", actor);
  if (noop && noop->elements > 0) {
    freeReplyObject(noop);
    return 1;
  }

  return 0;
}

void expire(redisContext *context, char *actor, char *suffix, int expiry)
{
  redisReply *ttl;

  ttl = redisCommand(context, "TTL %s:requests", actor);
  if (ttl && ttl->integer > 0) {
    redisCommand(context, "EXPIRE %s:%s %d", actor, suffix, ttl->integer);
    freeReplyObject(ttl);
  } else {
    redisCommand(context, "EXPIRE %s:%s %d", actor, suffix, expiry);
  }
}

void blacklist(redisContext *context, char *actor)
{
  redisCommand(context, "SET %s:repsheet:blacklist true", actor);
  redisCommand(context, "SADD repsheet:blacklist:history %s", actor);
}

void blacklist_and_expire(redisContext *context, int expiry, char *actor, char *message, int score)
{
  blacklist(context, actor);
  expire(context, actor, "repsheet:blacklist", expiry);

  if (score) {
    printf("Actor %s has been blacklisted: %s. [Score: %d]\n", actor, message, score);
  } else {
    printf("Actor %s has been blacklisted: %s\n", actor, message);
  }
}
