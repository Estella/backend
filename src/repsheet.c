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

#include "repsheet.h"
#include "score.h"
#include "report.h"
#include "blacklist.h"

config_t config;

redisContext *get_redis_context()
{
  redisContext *context;

  context = redisConnect(config.host, config.port);
  if (context == NULL || context->err) {
    if (context) {
      printf("Redis Connection Error: %s\n", context->errstr);
      redisFree(context);
    } else {
      perror("Connection Error: can't allocate redis context\n");
    }
    return NULL;
  } else {
    return context;
  }
}

static void print_usage()
{
  printf("Repsheet Backend Version %s\n", VERSION);
  printf("usage: repsheet [-h] [-p] [-sbr]\n  -h <redis host>\n  -p <redis port>\n  -s (score actors)\n  -r (report top 10 offenders)\n  -b (blacklist offenders)\n");
}

int main(int argc, char *argv[])
{
  int c;
  redisContext *context;

  config.host = "localhost";
  config.port = 6379;
  config.threshold = 200;
  config.score = 0;
  config.report = 0;
  config.blacklist = 0;
  config.expiry = (24 * 60 * 60);
  config.upstream = 0;

  while((c = getopt (argc, argv, "h:p:t:srbvu")) != -1)
    switch(c)
      {
      case 'h':
        config.host = optarg;
        break;
      case 'p':
        config.port = atoi(optarg);
        break;
      case 't':
        config.threshold = atoi(optarg);
        break;
      case 's':
        config.score = 1;
        break;
      case 'r':
        config.report = 1;
        break;
      case 'b':
        config.blacklist = 1;
        break;
      case 'u':
        config.upstream = 1;
        break;
      case 'v':
        print_usage();
        return 0;
        break;
      case '?':
        return 1;
      default:
        print_usage();
        abort();
      }

  context = get_redis_context();
  if (context == NULL) {
    return -1;
  }

  if (!config.report && !config.blacklist && !config.score && !config.upstream) {
    printf("No options specified, performing score operation.\nTo remove this message, specify -s (score) or [-r | -b | -u] (report, blacklist, or upstream)\n");
    score(context);
  }

  if (config.score) {
    score(context);
  }

  if (config.blacklist) {
    score(context);
    blacklist(context, config);
    score(context);
  }

  if (config.upstream) {
    publish_blacklist(context);
  }

  if (config.report) {
    score(context);
    report(context, config);
  }

  redisFree(context);

  return 0;
}
