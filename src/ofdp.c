#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <curl/curl.h>

#include "ofdp.h"

int ofdp_score(callback_buffer response)
{
  if (response.size == 0) {
    return 0;
  }

  xmlDocPtr doc;
  xmlXPathObjectPtr xpathObj;
  xmlNodeSetPtr nodes;
  xmlXPathContextPtr xpathCtx;
  int size, score = 0;

  xmlInitParser();

  doc = xmlParseMemory(response.buffer, response.size);

  xpathCtx = xmlXPathNewContext(doc);
  xpathObj = xmlXPathEvalExpression((xmlChar *)OFDP_SCORE_XPATH, xpathCtx);

  nodes = xpathObj->nodesetval;
  size = (nodes) ? nodes->nodeNr : 0;

  if (size == 0 || size > 1) {
    score = 0;
  } else {
    score = atoi((char *)xmlNodeGetContent(nodes->nodeTab[0]));
  }

  xmlXPathFreeContext(xpathCtx);
  xmlXPathFreeObject(xpathObj);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  xmlMemoryDump();

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
  // curl "http://wafsec.com/api?ip=<ip_address>

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
  char *address = malloc(16);
  redisReply *offenders, *score, *noop;

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
      }
    }
    freeReplyObject(offenders);
  }
}
