TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = test_runner.c    \
                      score_test.c     \
                      blacklist_test.c \
                      $(top_builddir)/src/util. h     \
                      $(top_builddir)/src/util.c      \
                      $(top_builddir)/src/score.c     \
                      $(top_builddir)/src/score.h     \
                      $(top_builddir)/src/report.c    \
                      $(top_builddir)/src/report.h    \
                      $(top_builddir)/src/blacklist.c \
                      $(top_builddir)/src/blacklist.h
test_runner_CFLAGS = $(CHECK_CFLAGS)
test_runner_LDADD = $(CHECK_LIBS) -lhiredis -lcurl -ljson
