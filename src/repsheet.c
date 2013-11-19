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
#include "analyze.h"
#include "upstream.h"
#include "cli.h"

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
  printf("usage: repsheet [-srabuv] [-h] [-p] [-e] [-t] [-o]\n \
 --score                  -s score actors\n \
 --report                 -r report top 10 offenders\n \
 --analyze                -a analyze and act on offenders\n \
 --publish                -u publish blacklist to upstream providers\n \
 --host                   -h <redis host>\n \
 --port                   -p <redis port>\n \
 --expiry                 -e <redis expiry> blacklist expire time\n \
 --modsecurity_threshold  -t <blacklist threshold>\n \
 --ofdp_threshold         -o <ofdp threshold> score and blacklist actors against wafsec.com\n \
 --version                -v print version and help\n");
}

int main(int argc, char *argv[])
{
  int c;
  long ofdp_threshold, blacklist_threshold, redis_port, redis_expiry;
  redisContext *context;

  config.score = 0;
  config.report = 0;
  config.analyze = 0;
  config.publish = 0;

  config.host = "localhost";
  config.port = 6379;
  config.expiry = TWENTYFOUR_HOURS;

  config.ofdp_threshold = 50;
  config.modsecurity_threshold = 200;

  static struct option long_options[] = {
    {"score",                 no_argument,       NULL, 's'},
    {"report",                no_argument,       NULL, 'r'},
    {"analyze",               no_argument,       NULL, 'a'},
    {"publish",               no_argument,       NULL, 'u'},

    {"host",                  required_argument, NULL, 'h'},
    {"port",                  required_argument, NULL, 'p'},
    {"expiry",                required_argument, NULL, 'e'},

    {"modsecurity_threshold", required_argument, NULL, 't'},
    {"ofdp_threshold",        required_argument, NULL, 'o'},

    {"version",               no_argument,       NULL, 'v'},
    {0,                       0,                 0,     0}
  };

  while((c = getopt_long(argc, argv, "h:p:e:t:o:srauv", long_options, NULL)) != -1)
    switch(c)
      {
      case 's':
        config.score = 1;
        break;
      case 'r':
        config.report = 1;
        break;
      case 'a':
        config.analyze = 1;
        break;
      case 'u':
        config.publish = 1;
        break;

      case 'h':
        config.host = optarg;
        break;
      case 'p':
        redis_port = process_command_line_argument(optarg);
        if (redis_port != INVALID_ARGUMENT_ERROR) {
          config.port = redis_port;
        } else {
          printf("Redis port must be between 1 and %d, defaulting to %d\n", USHRT_MAX, config.port);
        }
        break;
      case 'e':
        redis_expiry = process_command_line_argument(optarg);
        if (redis_expiry != INVALID_ARGUMENT_ERROR) {
          config.expiry = redis_expiry;
        } else {
          printf("Redis expiry must be between 1 and %d, defaulting to %d\n", USHRT_MAX, config.expiry);
        }
        break;

      case 't':
        blacklist_threshold = process_command_line_argument(optarg);
        if (blacklist_threshold != INVALID_ARGUMENT_ERROR) {
          config.modsecurity_threshold = blacklist_threshold;
        } else {
          printf("ModSecurity threshold must be between 1 and %d, defaulting to %d\n", USHRT_MAX, config.modsecurity_threshold);
        }
        break;
      case 'o':
        ofdp_threshold = process_command_line_argument(optarg);
        if (ofdp_threshold != INVALID_ARGUMENT_ERROR) {
          config.ofdp_threshold = ofdp_threshold;
        } else {
          printf("OFDP threshold must be between 1 and %d, defaulting to %d\n", USHRT_MAX, config.ofdp_threshold);
        }
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

  if (!config.score && !config.report && !config.analyze && !config.publish) {
    printf("No options specified, performing score operation\n");
    score(context);
  }

  if (config.score) {
    score(context);
  }

  if (config.report) {
    report(context);
  }

  if (config.analyze) {
    analyze(context, config);
  }

  if (config.publish) {
    publish_blacklist(context);
  }

  redisFree(context);

  return 0;
}
