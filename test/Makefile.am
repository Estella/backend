TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = test_runner.c    \
                      score_test.c     \
                      blacklist_test.c \
                      ofdp_test.c      \
                      ../src/util. h     \
                      ../src/util.c      \
                      ../src/score.c     \
                      ../src/score.h     \
                      ../src/report.c    \
                      ../src/report.h    \
                      ../src/blacklist.c \
                      ../src/blacklist.h \
                      ../src/ofdp.h      \
                      ../src/ofdp.c
test_runner_CFLAGS = $(CHECK_CFLAGS) $(JSON_C_CFLAGS) $(LIBCURL_CFLAGS) $(XML2_CFLAGS)
test_runner_LDADD = $(CHECK_LIBS) $(JSON_C_LIBS) $(LIBCURL_LIBS) $(XML2_LIBS) -lhiredis
