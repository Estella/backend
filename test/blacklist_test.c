#include "../src/blacklist.h"
#include "../src/score.h"
#include "test_suite.h"

redisContext *context;
redisReply *reply;
config_t config;

void blacklist_setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }

  config.threshold = 1;
  config.expiry = (24 * 60 * 60);
}

void blacklist_teardown(void)
{
  redisCommand(context, "flushdb");

  if (reply) {
    freeReplyObject(reply);
  }

  redisFree(context);
}

START_TEST(does_nothing_if_there_are_no_offenders)
{
  score(context);
  blacklist(context, config);

  reply = redisCommand(context, "KEYS *:repsheet:blacklist");
  ck_assert_int_eq(reply->elements, 0);
}
END_TEST

START_TEST(does_not_delete_offenders_key)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");

  score(context);
  blacklist(context, config);

  reply = redisCommand(context, "EXISTS offenders");
  ck_assert_int_eq(reply->integer, 1);
}
END_TEST

START_TEST(does_not_blacklist_whitelisted_actors)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");
  redisCommand(context, "ZINCRBY 1.1.1.2:detected 10 950001");
  redisCommand(context, "SET 1.1.1.1:repsheet:whitelist true");

  score(context);
  blacklist(context, config);

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

  score(context);
  blacklist(context, config);

  reply = redisCommand(context, "EXISTS 1.1.1.1:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.2:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.3:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);

  reply = redisCommand(context, "EXISTS 1.1.1.4:repsheet:blacklist");
  ck_assert_int_eq(reply->integer, 1);
}
END_TEST

START_TEST(expires_blacklist_keys)
{
  redisCommand(context, "ZINCRBY 1.1.1.1:detected 10 950001");

  score(context);
  blacklist(context, config);

  reply = redisCommand(context, "TTL 1.1.1.1:repsheet:blacklist");
  ck_assert(reply->integer > 86300);
}
END_TEST

Suite *make_blacklist_suite(void) {
  Suite *suite = suite_create("blacklist");

  TCase *tc_blacklist = tcase_create("units");
  tcase_add_checked_fixture(tc_blacklist, blacklist_setup, blacklist_teardown);
  tcase_add_test(tc_blacklist, does_nothing_if_there_are_no_offenders);
  tcase_add_test(tc_blacklist, does_not_delete_offenders_key);
  tcase_add_test(tc_blacklist, does_not_blacklist_whitelisted_actors);
  tcase_add_test(tc_blacklist, expires_blacklist_keys);
  tcase_add_test(tc_blacklist, can_blacklist_multiple_offenders_at_once);
  suite_add_tcase(suite, tc_blacklist);

  return suite;
}
