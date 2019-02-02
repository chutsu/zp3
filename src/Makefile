CC=g++ -std=c++11
CFLAGS=-I../deps/ssd1306/src
LIBS=-lmpg123 \
	-lao \
	-ltag \
	-lssd1306 \
	-lssd1306_sdl \
	-L../deps/ssd1306/bld \
	-lSDL2 \
	-lpthread

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# ARCHIVER
AR = ar
ARFLAGS = rvs

COMPILE_OBJ = \
	@echo "CC [${@:.o=.cpp}]"; \
	$(CC) $(CFLAGS) -c ${@:.o=.cpp} -o $@

MAKE_TEST = \
	@echo "CC [$@.cpp]"; \
	$(CC) $(CFLAGS) $@.cpp -o $@ -L. -lzp3 $(LIBS)

all: test_music test_player

clean:
	@rm -f zp3
	@rm -f *.o

%.o: %.cpp
	$(COMPILE_OBJ)

libzp3: util.o music.o display.o player.o
	$(AR) $(ARFLAGS) $@.a $?

test_music: libzp3
	$(MAKE_TEST)

test_player: libzp3
	$(MAKE_TEST)

# zp3: music.o util.o display.o
# 	@$(CC) $(CFLAGS) -c $@.cpp -o $@.o
# 	$(CC) $(CFLAGS) $@.o $? -o $@ $(DEPS);
