#ifndef __REPSHEET_H
#define __REPSHEET_H

#define VERSION "0.2"

typedef struct config_t {
  char *host;
  int port;
  int threshold;
  int report;
  int blacklist;
  int score;
  int expiry;
} config_t;

#endif
