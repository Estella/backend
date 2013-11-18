#include "../src/score.h"
#include "../src/analyze.h"
#include "test_suite.h"

redisContext *context;
redisReply *reply;
config_t config;

void analyze_setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }

  config.modsecurity_threshold = 10;
  config.expiry = (24 * 60 * 60);
}

void analyze_teardown(void)
{
  redisCommand(context, "flushdb");

  if (reply) {
    freeReplyObject(reply);
  }

  redisFree(context);
}

START_TEST(does_nothing_if_there_are_no_offenders)
{
  analyze(context, config);

  reply = redisCommand(context, "KEYS *:repsheet:blacklist");
  ck_assert_int_eq(reply->elements, 0);
}
END_TEST

START_TEST(does_not_blacklist_whitelisted_actors)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.2:detected 10 950001");
  redisCommand(context, "SET 1.1.1.1:repsheet:whitelist true");

  analyze(context, config);

  reply = redisCommand(context, "GET 1.1.1.2:repsheet:blacklist");
  ck_assert_int_eq(reply->type, REDIS_REPLY_STRING);

  reply = redisCommand(context, "EXISTS 1.1.1.1:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);
}
END_TEST

START_TEST(can_blacklist_multiple_offenders_at_once)
{
  reply = redisCommand(context, "EXISTS 1.1.1.1:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);

  reply = redisCommand(context, "EXISTS 1.1.1.2:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);

  reply = redisCommand(context, "EXISTS 1.1.1.3:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);

  reply = redisCommand(context, "EXISTS 1.1.1.4:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);

  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.2:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.3:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.4:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.5:detected 5 950001");
  redisCommand(context, "ZINCRBY 1.1.1.6:detected 7 950001");

  analyze(context, config);

  reply = redisCommand(context, "EXISTS 1.1.1.1:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.2:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.3:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.4:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.5:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);

  reply = redisCommand(context, "EXISTS 1.1.1.6:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 0);
}
END_TEST

START_TEST(expires_blacklisted_keys)
{
  redisCommand(context, "ZINCRBY 1.1.1.5:detected 10 950001");

  analyze(context, config);

  reply = redisCommand(context, "TTL 1.1.1.5:repsheet:blacklist");
  ck_assert(reply->integer > 86300);
}
END_TEST

START_TEST(keeps_a_record_of_blacklisted_actors)
{
  redisCommand(context, "ZINCRBY 1.1.1.6:detected 10 950001");

  analyze(context, config);

  reply = redisCommand(context, "SISMEMBER repsheet:blacklist:history 1.1.1.6");
  ck_assert_int_eq(reply->integer, 1);
}
END_TEST

START_TEST(blacklists_historical_repeat_offenders)
{
  redisCommand(context, "SADD repsheet:blacklist:history 1.1.1.7");
  redisCommand(context, "ZINCRBY 1.1.1.7:detected 1 950001");

  analyze(context, config);

  reply = redisCommand(context, "EXISTS 1.1.1.7:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);
}
END_TEST

Suite *make_analyze_suite(void) {
  Suite *suite = suite_create("analyze");

  TCase *tc_analyze = tcase_create("units");
  tcase_add_checked_fixture(tc_analyze, analyze_setup, analyze_teardown);
  tcase_add_test(tc_analyze, does_nothing_if_there_are_no_offenders);
  tcase_add_test(tc_analyze, does_not_blacklist_whitelisted_actors);
  tcase_add_test(tc_analyze, can_blacklist_multiple_offenders_at_once);
  tcase_add_test(tc_analyze, expires_blacklisted_keys);
  tcase_add_test(tc_analyze, keeps_a_record_of_blacklisted_actors);
  tcase_add_test(tc_analyze, blacklists_historical_repeat_offenders);
  suite_add_tcase(suite, tc_analyze);

  return suite;
}
