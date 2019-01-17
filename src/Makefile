CC=g++
CFLAGS=-I/usr/include
LIBS=-lmpg123 -lao -ltag -lpthread

all: zp3

clean:
	@rm -f zp3
	@rm -f *.o

zp3: clean
	@$(CC) $(CFLAGS) -c $@.cpp -o $@.o
	@$(CC) $(CFLAGS) $@.o -o $@ $(LIBS);
