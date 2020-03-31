CFLAGS=-Wall
LDFLAGS=
CC=gcc

SRC=ut_test.c ut.c


.PHONY:	all

all:	ut_test

clean:
	rm -f ut_test $(SRC:.c=.o)

ut_test: $(SRC:.c=.o)
	$(CC) $(CFLAGS) -o $@ $(SRC:.c=.o) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
