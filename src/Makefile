CC=g++
CFLAGS=-I../deps/ssd1306/src
LIBS=-lmpg123 \
		 -lao \
		 -ltag \
		 -lssd1306 \
		 -lssd1306_sdl \
		 -L../deps/ssd1306/bld \
		 -lSDL2 \
		 -lpthread

all: zp3

clean:
	@rm -f zp3
	@rm -f *.o

zp3: clean
	@$(CC) $(CFLAGS) -c $@.cpp -o $@.o
	@$(CC) $(CFLAGS) $@.o -o $@ $(LIBS);
