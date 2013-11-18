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

#ifndef __REPSHEET_H
#define __REPSHEET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <curl/curl.h>
#include <json/json.h>

#include "hiredis/hiredis.h"

#include "util.h"
#include "score.h"

#define VERSION "1.5.0"

#define DEPRECATED(arg, message) printf("The %s argument has been deprecated. %s\n", arg, message);
#define TWENTYFOUR_HOURS 86400;

typedef struct config_t {
  // top level options
  int score;
  int report;
  int analyze;
  int publish;

  // redis configuration options
  int port;
  char *host;
  int expiry;

  // analyze options
  int blacklist;
  int ofdp;

  // thresholds
  int ofdp_threshold;
  int modsecurity_threshold;
} config_t;

#endif
