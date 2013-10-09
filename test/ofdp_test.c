#include <stdlib.h>
#include "../src/repsheet.h"
#include "../src/upstream.h"
#include "../src/ofdp.h"
#include "test_suite.h"

callback_buffer data;

void ofdp_setup(void)
{
  data.buffer = "<?xml version=\"1.0\" ?> \
<wafsec> \
  <score>296</score> \
  <ipinfo> \
    <ip>216.218.134.12</ip> \
    <hostname>tor-exit.altsci.com</hostname> \
  </ipinfo> \
  <geoloc> \
    <country>US</country> \
    <city>Fremont</city> \
    <latitude>37.515499</latitude> \
    <longitude>-121.896202</longitude> \
    <autonomous_system_number>AS6939</autonomous_system_number> \
    <autonomous_system_name>Hurricane Electric, Inc.</autonomous_system_name> \
  </geoloc> \
  <details> \
    <proxy_factor>0</proxy_factor> \
    <tor_factor>3</tor_factor> \
    <honeypot_score_factor>29</honeypot_score_factor> \
    <malware_score_factor>0</malware_score_factor> \
    <spam_score_factor>264</spam_score_factor> \
    <high_risk_country_factor>0</high_risk_country_factor> \
  </details> \
</wafsec>";
  data.size = strlen(data.buffer) + 1;
}

void ofdp_teardown(void)
{
  data.buffer = NULL;
  data.size = 0;
}

START_TEST(properly_finds_the_score_in_the_response)
{
  int score = ofdp_score(data);
  ck_assert_int_eq(score, 296);
}
END_TEST

Suite *make_ofdp_suite(void) {
  Suite *suite = suite_create("ofdp");

  TCase *tc_ofdp = tcase_create("units");
  tcase_add_checked_fixture(tc_ofdp, ofdp_setup, ofdp_teardown);
  tcase_add_test(tc_ofdp, properly_finds_the_score_in_the_response);
  suite_add_tcase(suite, tc_ofdp);

  return suite;
}
