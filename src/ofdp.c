#include <stdio.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

int main(void)
{
  xmlDocPtr doc;
  xmlXPathObjectPtr xpathObj;
  xmlNodeSetPtr nodes;
  xmlXPathContextPtr xpathCtx;
  const char *query = "/wafsec/score";
  int size;

  xmlInitParser();
  
  doc = xmlParseFile("sample.xml");
  
  xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == NULL) {
    printf("Error: unable to create new XPath context\n");
    return -1;
  }

  xpathObj = xmlXPathEvalExpression((xmlChar *)query, xpathCtx);
  if (xpathObj == NULL) {
    printf("Error: unable to evaluate xpath expression \"%s\"\n", query);
    xmlXPathFreeContext(xpathCtx);
    return -1;
  }
  
  nodes = xpathObj->nodesetval;
  size = (nodes) ? nodes->nodeNr : 0;

  if (size == 0) {
    return -1;
  }

  if (size == 1) {
    printf("%s\n", xmlNodeGetContent(nodes->nodeTab[0]));
  }

  xmlXPathFreeContext(xpathCtx);
  xmlXPathFreeObject(xpathObj);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  xmlMemoryDump();

  return 0;
}
//gcc -Wall test.c -o wafsec -I/usr/include/libxml2 -lxml2
