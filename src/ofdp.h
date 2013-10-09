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

#ifndef __OFDP_H
#define __OFDP_H

#include <curl/curl.h>
#include "util.h"
#include "repsheet.h"

#define OFDP_URL "http://wafsec.com/api?ip="
#define OFDP_SCORE_XPATH "/wafsec/score"

int ofdp_score(callback_buffer response);
callback_buffer ofdp_lookup(char *address);
void ofdp_lookup_offenders(redisContext *context);

#endif
