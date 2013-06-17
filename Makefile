CFLAGS += -Wall -Werror
BIN = repsheet

.PHONY: clean

all: $(BIN)

repsheet.o:
	gcc $(CFLAGS) -c repsheet.c

repsheet: repsheet.o
	gcc repsheet.o -o $(BIN) -lhiredis

clean:
	rm -rf *.o *.dSYM repsheet
