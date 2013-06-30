TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = repsheet_test.c \
                      test_runner.c \
                      $(top_builddir)/src/score.c     \
                      $(top_builddir)/src/score.h     \
                      $(top_builddir)/src/report.c    \
                      $(top_builddir)/src/report.h    \
                      $(top_builddir)/src/blacklist.c \
                      $(top_builddir)/src/blacklist.h
test_runner_CFLAGS = $(CHECK_CFLAGS)
test_runner_LDADD = $(CHECK_LIBS) -lhiredis
