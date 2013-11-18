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

#ifndef __UTIL_H
#define __UTIL_H

#include "repsheet.h"

#define THRESHOLD_MESSAGE "The actor has exceeded the ModSecurity blacklist threshold"
#define HISTORY_MESSAGE "The actor is a return offender"
#define OFDP_MESSAGE "The actor has exceeded the OFDP risk threshold"

typedef struct callback_buffer
{
  char * buffer;
  size_t size;
} callback_buffer;

char *strip_address(char *key);
int no_action_required(redisContext *context, char *actor);
void expire(redisContext *context, char *actor, char *suffix, int expiry);
void blacklist(redisContext *context, char *actor);
void blacklist_and_expire(redisContext *context, int expiry, char *actor, char *message, int score);

#endif
