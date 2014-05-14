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

#ifndef __UPSTREAM_H
#define __UPSTREAM_H

#include "backend.h"

#define EMAIL getenv("CLOUDFLARE_EMAIL")
#define TOKEN getenv("CLOUDFLARE_TOKEN")
#define CLOUDFLARE_URL "https://www.cloudflare.com/api_json.html"

void publish_blacklist(redisContext *context);

#endif
