prefix=/usr/local
bindir=$(prefix)/bin
sharedir=$(prefix)/share
mandir=$(sharedir)/man
man1dir=$(mandir)/man1

CFLAGS += -Wall -Werror

all: repsheet

repsheet: repsheet.o
	gcc repsheet.o -o repsheet -lhiredis

repsheet.o: repsheet.c
	gcc $(CFLAGS) -c repsheet.c

clean:
	rm -rf *.o *.dSYM repsheet

install: all
	install repsheet $(DESTDIR)$(bindir)

.PHONY: clean