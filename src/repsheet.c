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

#include "backend.h"
#include "report.h"
#include "analyze.h"
#include "cli.h"

config_t config;

static void print_usage()
{
  printf("Repsheet Backend Version %s\n", VERSION);
  printf("usage: repsheet [-srauv] [-h] [-p] [-e] [-t] [-o]\n \
 --score                  -s score actors\n \
 --report                 -r report top 10 offenders\n \
 --analyze                -a analyze and act on offenders\n \
 --host                   -h <redis host>\n \
 --port                   -p <redis port>\n \
 --expiry                 -e <redis expiry> blacklist expire time\n \
 --modsecurity_threshold  -t <blacklist threshold>\n \
 --version                -v print version and help\n");
}

int main(int argc, char *argv[])
{
  int c;
  long blacklist_threshold, redis_port, redis_expiry;
  redisContext *context;

  config.score = 0;
  config.report = 0;
  config.analyze = 0;

  config.host = "localhost";
  config.port = 6379;
  config.expiry = TWENTYFOUR_HOURS;

  config.modsecurity_threshold = 200;

  static struct option long_options[] = {
    {"score",                 no_argument,       NULL, 's'},
    {"report",                no_argument,       NULL, 'r'},
    {"analyze",               no_argument,       NULL, 'a'},

    {"host",                  required_argument, NULL, 'h'},
    {"port",                  required_argument, NULL, 'p'},
    {"expiry",                required_argument, NULL, 'e'},

    {"modsecurity_threshold", required_argument, NULL, 't'},

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

  context = get_redis_context(config.host, config.port, 0);
  if (context == NULL) {
    return -1;
  }

  if (!config.score && !config.report && !config.analyze) {
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

  redisFree(context);

  return 0;
}
