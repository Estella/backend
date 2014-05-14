#include "../src/backend.h"
#include "../src/cli.h"
#include "test_suite.h"

START_TEST(returns_invalid_if_argument_is_equal_to_zero)
{
  long result = process_command_line_argument("0");
  ck_assert(result == INVALID_ARGUMENT_ERROR);
}
END_TEST

START_TEST(returns_invalid_if_argument_is_less_than_zero)
{
  long result = process_command_line_argument("-1");
  ck_assert(result == INVALID_ARGUMENT_ERROR);
}
END_TEST

START_TEST(returns_invalid_if_argument_is_greater_than_ushrt_max)
{
  long result = process_command_line_argument("65536");
  ck_assert(result == INVALID_ARGUMENT_ERROR);
}
END_TEST

START_TEST(returns_the_processed_value_when_it_is_in_bounds);
{
  long result = process_command_line_argument("65535");
  ck_assert(result == 65535);
  result = process_command_line_argument("1");
  ck_assert(result == 1);
}
END_TEST


Suite *make_cli_suite(void) {
  Suite *suite = suite_create("cli");

  TCase *tc_cli = tcase_create("process command line arguments");
  tcase_add_test(tc_cli, returns_invalid_if_argument_is_equal_to_zero);
  tcase_add_test(tc_cli, returns_invalid_if_argument_is_less_than_zero);
  tcase_add_test(tc_cli, returns_invalid_if_argument_is_greater_than_ushrt_max);
  tcase_add_test(tc_cli, returns_the_processed_value_when_it_is_in_bounds);
  suite_add_tcase(suite, tc_cli);

  return suite;
}
