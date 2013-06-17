CFLAGS += -Wall -Werror

.PHONY: clean

repsheet: repsheet.o
	gcc repsheet.o -o repsheet -lhiredis

repsheet.o: repsheet.c
	gcc $(CFLAGS) -c repsheet.c

clean:
	rm -rf *.o *.dSYM repsheet
