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
