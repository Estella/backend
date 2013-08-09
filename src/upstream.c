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

#include "util.h"
#include "upstream.h"

static int success(callback_buffer *response)
{
  struct json_object *json;
  const char *status;

  json = json_tokener_parse(response->buffer);
  status = json_object_get_string(json_object_object_get(json, "result"));

  if (status && strcasecmp(status, "success") == 0) {
    return 1;
  }

  return 0;
}

static size_t cloudflare_blacklist_callback(void *ptr, size_t size, size_t nmemb, void *data)
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

static void cloudflare_blacklist(char *address)
{
  /* curl https://www.cloudflare.com/api_json.html \ */
  /*   -d 'a=ban' \ */
  /*   -d 'tkn=token' \ */
  /*   -d 'email=example@example.com' \ */
  /*   -d 'key=1.1.1.1' */

  CURL *curl;
  CURLcode res;

  struct curl_httppost *post=NULL;
  struct curl_httppost *ptr=NULL;
  struct curl_slist *headerlist=NULL;

  callback_buffer response;
  response.buffer = NULL;
  response.size = 0;

  curl_global_init(CURL_GLOBAL_ALL);

  if (!EMAIL) {
    printf("You must set the CLOUDFLARE_EMAIL environment variable\n");
    exit(1);
  }

  if (!TOKEN) {
    printf("You must set the CLOUDFLARE_TOKEN environment variable\n");
    exit(1);
  }

  curl_formadd(&post, &ptr, CURLFORM_COPYNAME, "a",     CURLFORM_COPYCONTENTS, "ban",     CURLFORM_END);
  curl_formadd(&post, &ptr, CURLFORM_COPYNAME, "tkn",   CURLFORM_COPYCONTENTS, TOKEN,     CURLFORM_END);
  curl_formadd(&post, &ptr, CURLFORM_COPYNAME, "email", CURLFORM_COPYCONTENTS, EMAIL,     CURLFORM_END);
  curl_formadd(&post, &ptr, CURLFORM_COPYNAME, "key",   CURLFORM_COPYCONTENTS, address, CURLFORM_END);

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, CLOUDFLARE_URL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cloudflare_blacklist_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    if (!success(&response)) {
      printf("Upstream processing failed\n");
    }

    curl_easy_cleanup(curl);
    curl_formfree(post);
    curl_slist_free_all (headerlist);
  }
}

void publish_blacklist(redisContext *context) {
  int i;
  redisReply *blacklist;
  char *address = malloc(16);

  blacklist = redisCommand(context, "KEYS *:repsheet:blacklist");
  if (blacklist && blacklist->type == REDIS_REPLY_ARRAY) {
    for(i = 0; i < blacklist->elements; i++) {
      address = strip_address(blacklist->element[i]->str);
      cloudflare_blacklist(address);
    }
    freeReplyObject(blacklist);
  }

  free(address);
}
