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

#define VERSION "0.7"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "hiredis/hiredis.h"

typedef struct config_t {
  char *host;
  int port;
  int threshold;
  int report;
  int blacklist;
  int score;
  int expiry;
} config_t;

#endif
