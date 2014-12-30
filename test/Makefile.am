TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = test_runner.c      \
                      score_test.c       \
                      analyze_test.c     \
                      cli_test.c         \
                      ../src/util. h     \
                      ../src/util.c      \
                      ../src/score.c     \
                      ../src/score.h     \
                      ../src/report.c    \
                      ../src/report.h    \
                      ../src/analyze.c   \
                      ../src/analyze.h   \
                      ../src/cli.h       \
                      ../src/cli.c
test_runner_CFLAGS = $(check_CFLAGS)
test_runner_LDADD = $(check_LIBS) -lhiredis -lrepsheet
