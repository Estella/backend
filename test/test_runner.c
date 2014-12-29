#include <stdlib.h>
#include "test_suite.h"

int main(void) {
  SRunner *runner = srunner_create(make_analyze_suite());
  srunner_add_suite(runner, make_score_suite());
  srunner_add_suite(runner, make_ofdp_suite());
  srunner_add_suite(runner, make_cli_suite());
  srunner_run_all(runner, CK_VERBOSE);
  int number_failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
