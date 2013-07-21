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
  redisCommand(context, "flushdb");

  if (reply) {
    freeReplyObject(reply);
  }

  redisFree(context);
}

START_TEST(does_nothing_if_there_are_no_suspects)
{
  redisCommand(context, "DEL offenders");

  score(context);

  reply = redisCommand(context, "EXISTS offenders");
  ck_assert_int_eq(reply->integer, 0);
}
END_TEST

START_TEST(creates_offenders_key_of_type_zset)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");

  score(context);

  reply = redisCommand(context, "TYPE offenders");
  ck_assert_str_eq(reply->str, "zset");
}
END_TEST

START_TEST(does_not_score_blacklisted_actors)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.2:detected 10 950001");
  redisCommand(context, "SET 1.1.1.2:repsheet:blacklist true");

  score(context);

  reply = redisCommand(context, "ZRANGE offenders 0 -1");
  ck_assert_int_eq(reply->elements, 1);
  ck_assert_str_eq(reply->element[0]->str, "1.1.1.1");
}
END_TEST

START_TEST(does_not_score_whitelisted_actors)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.2:detected 10 950001");
  redisCommand(context, "SET 1.1.1.2:repsheet:whitelist true");

  score(context);

  reply = redisCommand(context, "ZRANGE offenders 0 -1");
  ck_assert_int_eq(reply->elements, 1);
  ck_assert_str_eq(reply->element[0]->str, "1.1.1.1");
}
END_TEST

START_TEST(accounts_for_all_offenses_of_an_actor)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 100 950001");

  score(context);

  reply = redisCommand(context, "ZSCORE offenders 1.1.1.1");
  ck_assert_str_eq(reply->str, "110");
}
END_TEST

Suite *make_score_suite(void) {
  Suite *suite = suite_create("score");

  TCase *tc_score = tcase_create("units");
  tcase_add_checked_fixture(tc_score, setup, teardown);
  tcase_add_test(tc_score, does_nothing_if_there_are_no_suspects);
  tcase_add_test(tc_score, creates_offenders_key_of_type_zset);
  tcase_add_test(tc_score, does_not_score_blacklisted_actors);
  tcase_add_test(tc_score, does_not_score_whitelisted_actors);
  tcase_add_test(tc_score, accounts_for_all_offenses_of_an_actor);
  suite_add_tcase(suite, tc_score);

  return suite;
}
