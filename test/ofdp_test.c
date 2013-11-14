#include <stdlib.h>
#include "../src/repsheet.h"
#include "../src/upstream.h"
#include "../src/ofdp.h"
#include "test_suite.h"

callback_buffer data;

void ofdp_setup(void)
{
  data.buffer = "{\"asname\":\"Hurricane Electric, Inc.\",\"asnumber\":\"AS6939\",\"hostname\":\"tor-exit.altsci.com\",\"ip\":\"216.218.134.12\",\"score\":\"34\"}";
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
  ck_assert_int_eq(score, 34);
}
END_TEST

START_TEST(handles_non_json_response)
{
  data.buffer = "bad response";
  data.size = strlen(data.buffer) + 1;
  int score = ofdp_score(data);
}
END_TEST

START_TEST(handles_no_score_in_json_response)
{
  data.buffer = "{\"foo\":\"bar\"}";
  data.size = strlen(data.buffer) + 1;
  int score = ofdp_score(data);
}
END_TEST

Suite *make_ofdp_suite(void) {
  Suite *suite = suite_create("ofdp");

  TCase *tc_ofdp = tcase_create("units");
  tcase_add_checked_fixture(tc_ofdp, ofdp_setup, ofdp_teardown);
  tcase_add_test(tc_ofdp, properly_finds_the_score_in_the_response);
  tcase_add_test(tc_ofdp, handles_non_json_response);
  tcase_add_test(tc_ofdp, handles_no_score_in_json_response);
  suite_add_tcase(suite, tc_ofdp);

  return suite;
}
