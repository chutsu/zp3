CC=g++
CFLAGS=-I/usr/include
LIBS=-lmpg123 -lao -lpthread

all: zp3

clean:
	@echo "Cleaning *.o"
	@rm -f zp3
	@rm -f *.o

zp3: clean
	$(CC) $(CFLAGS) -c $@.c -o $@.o
	$(CC) $(CFLAGS) $@.o -o $@ $(LIBS);
