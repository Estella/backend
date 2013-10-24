#include "ofdp.h"

int ofdp_score(callback_buffer response)
{
  struct json_object *json;
  long score;

  json = json_tokener_parse(response.buffer);
  score = strtol(json_object_get_string(json_object_object_get(json, "score")), 0, 10);

  if (errno == ERANGE || score <= 0 || score > USHRT_MAX) {
    return 0;
  }

  return score;
}

static size_t ofdp_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  callback_buffer *mem = (callback_buffer *)data;

  mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);

  if (mem->buffer) {
    memcpy(&(mem->buffer[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;
  }

  return realsize;
}

callback_buffer ofdp_lookup(char *address)
{
  CURL *curl;
  CURLcode res;
  callback_buffer response;
  char *full_url;

  response.buffer = NULL;
  response.size = 0;

  full_url = malloc(strlen(OFDP_URL) + strlen(address) + 1);
  sprintf(full_url, "%s%s", OFDP_URL, address);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ofdp_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }

  free(full_url);
  return response;
}

void ofdp_lookup_offenders(redisContext *context, config_t config)
{
  int i, wafsec_score;
  redisReply *offenders, *score, *noop, *ttl;

  offenders = redisCommand(context, "ZRANGEBYSCORE offenders 0 +inf");
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for (i = 0; i < offenders->elements; i++) {
      noop = redisCommand(context, "KEYS %s:repsheet:*", offenders->element[i]->str);
      if (noop && (noop->elements > 0)) {
        freeReplyObject(noop);
        continue;
      }

      score = redisCommand(context, "GET %s:score", offenders->element[i]->str);
      if (score && (score->type != REDIS_REPLY_NIL)) {
        freeReplyObject(score);
        continue;
      }

      wafsec_score = ofdp_score(ofdp_lookup(offenders->element[i]->str));
      redisCommand(context, "SET %s:score %d", offenders->element[i]->str, wafsec_score);

      if (wafsec_score > config.ofdp_threshold) {

        redisCommand(context, "SET %s:repsheet:blacklist true", offenders->element[i]->str);
        printf("Actor %s has been blacklisted due to high OFDP risk (Score: %d)\n", offenders->element[i]->str, wafsec_score);

        ttl = redisCommand(context, "TTL %s:requests", offenders->element[i]->str);
        if (ttl && ttl->integer > 0) {
          redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", offenders->element[i]->str, ttl->integer);
          freeReplyObject(ttl);
        } else {
          redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", offenders->element[i]->str, config.expiry);
        }
      }
    }
    freeReplyObject(offenders);
  }
}
