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

#include "analyze.h"

void analyze(redisContext *context, config_t config)
{
  score(context);

  int i, modsecurity_score, ofdp_score;
  int printed = 0;
  redisReply *offenders;

  offenders = redisCommand(context, "ZRANGE offenders 0 -1 WITHSCORES");
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for(i = 0; i < offenders->elements; i += 2) {
      if (no_action_required(context, offenders->element[i]->str)) {
        continue;
      }

      if(is_historical_offender(context, offenders->element[i]->str)) {
        blacklist_and_expire(context, offenders->element[i]->str, config.expiry, "Return Offender");
        continue;
      }

      modsecurity_score = strtol(offenders->element[i+1]->str, 0, 10);
      if (modsecurity_score >= config.modsecurity_threshold) {
        blacklist_and_expire(context, offenders->element[i]->str ,config.expiry, "ModSecurity Threshold");
        continue;
      }

      if (!is_previously_scored(context, offenders->element[i]->str)) {
        ofdp_score = lookup_and_store_ofdp_score(context, offenders->element[i]->str, config.expiry);
        if (ofdp_score > config.ofdp_threshold) {
          blacklist_and_expire(context, offenders->element[i]->str, config.expiry, "OFDP Threshold");
        }
      }
    }
    freeReplyObject(offenders);
  }

  score(context);
}
