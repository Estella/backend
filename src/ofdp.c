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
  xmlDocPtr doc;
  xmlXPathObjectPtr xpathObj;
  xmlNodeSetPtr nodes;
  xmlXPathContextPtr xpathCtx;
  const char *query = "/wafsec/score";
  int size, score;

  xmlInitParser();

  doc = xmlParseMemory(response.buffer, response.size);

  xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == NULL) {
    printf("Error: unable to create new XPath context\n");
    return 0;
  }

  xpathObj = xmlXPathEvalExpression((xmlChar *)query, xpathCtx);
  if (xpathObj == NULL) {
    printf("Error: unable to evaluate xpath expression \"%s\"\n", query);
    xmlXPathFreeContext(xpathCtx);
    return 0;
  }

  nodes = xpathObj->nodesetval;
  size = (nodes) ? nodes->nodeNr : 0;

  if (size == 0) {
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
  response.buffer = NULL;
  response.size = 0;
  char *full_url;

  full_url = malloc(strlen(OFDP_URL) + strlen(address) + 1);
  sprintf(full_url, "%s%s", OFDP_URL, address);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ofdp_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      printf("curl failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }

  return response;
}

/*
int main(int argc, char *argv[])
{
  int score = ofdp_score(ofdp_lookup(argv[1]));
  printf("%d\n", score);
  return 0;
}
*/

//gcc -Wall ofdp.c -o ofdp -I/usr/include/libxml2 -lxml2 -lcurl
