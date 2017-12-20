CC = gcc
CFLAGS = -Wall -g
LIB = -lbsd

all: virtmem

benchmark: CFLAGS += -DBENCHMARK
benchmark: virtmem

virtmem: main.o page_table.o disk.o program.o
	$(CC) $+ -o $@ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o virtmem
