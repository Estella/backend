#include "ofdp.h"

int ofdp_score(callback_buffer response)
{
  struct json_object *json, *score_obj;
  long score;

  json = json_tokener_parse(response.buffer);

  if (is_error(json)) {
    printf("Error parsing JSON response. The server responded with:\n\n%s\n\n", response.buffer);
    return 0;
  }

  score_obj = json_object_object_get(json, "score");
  if (!score_obj) {
    printf("Could not locate score in response. The server responded with:\n\n%s\n\n", response.buffer);
    return 0;
  }

  score = strtol(json_object_get_string(score_obj), 0, 10);

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

static callback_buffer ofdp_lookup(char *address)
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

static int lookup_and_store_ofdp_score(redisContext *context, char *actor, int expiry)
{
  int score;

  score = ofdp_score(ofdp_lookup(actor));
  redisCommand(context, "SET %s:score %d", actor, score);
  expire(context, actor, "score", expiry);

  return score;
}

static int previously_scored(redisContext *context, char *actor)
{
  redisReply *score;

  score = redisCommand(context, "GET %s:score", actor);
  if (score && (score->type != REDIS_REPLY_NIL)) {
    freeReplyObject(score);
    return 1;
  }

  return 0;
}

void ofdp_lookup_offenders(redisContext *context, config_t config)
{
  int i, score;
  redisReply *offenders;

  offenders = redisCommand(context, "ZRANGEBYSCORE offenders 0 +inf");
  if (offenders && (offenders->type == REDIS_REPLY_ARRAY)) {
    for (i = 0; i < offenders->elements; i++) {
      if (no_action_required(context, offenders->element[i]->str) || previously_scored(context, offenders->element[i]->str)) {
        continue;
      }

      score = lookup_and_store_ofdp_score(context, offenders->element[i]->str, config.expiry);

      if (score > config.ofdp_threshold) {
        blacklist_and_expire(context, config.expiry, offenders->element[i]->str, OFDP_MESSAGE, score);
      }
    }
    freeReplyObject(offenders);
  }
}
