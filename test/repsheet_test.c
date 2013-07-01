#include "../src/score.h"
#include "test_suite.h"

redisContext *context;
redisReply *reply;

void setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }
}

void teardown(void)
{
  freeReplyObject(redisCommand(context, "flushdb"));
  if (!reply == NULL) {
    freeReplyObject(reply);
  }
  redisFree(context);
}

START_TEST(score_creates_offenders_key_of_type_zset)
{
  freeReplyObject(redisCommand(context, "set 1.1.1.1:950001:count 10"));
  score(context);
  reply = redisCommand(context, "type offenders");
  ck_assert_str_eq(reply->str, "zset");
}
END_TEST

Suite *make_repsheet_suite(void) {
  Suite *suite = suite_create("backend");

  TCase *tc_backend = tcase_create("repsheet_backend");
  tcase_add_checked_fixture(tc_backend, setup, teardown);
  tcase_add_test(tc_backend, score_creates_offenders_key_of_type_zset);
  suite_add_tcase(suite, tc_backend);

  return suite;
}
