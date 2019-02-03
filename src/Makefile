# FILES AND DIRS
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
TESTS = test_music test_player test_display
BIN_DIR = ../bin

# COMPILER SETTINGS
CC=g++ -std=c++11 -Wall -g
CFLAGS=-I../deps/ssd1306/src
# LIBS=-lmpg123 \
# 	-lao \
# 	-ltag \
# 	-L../deps/ssd1306/bld \
# 	-lssd1306 \
# 	-lpthread
LIBS=-lmpg123 \
	-lao \
	-ltag \
	-L../deps/ssd1306/bld \
	-lssd1306 \
	-lssd1306_sdl \
	-lSDL2 \
	-lpthread

# ARCHIVER SETTTINGS
AR = ar
ARFLAGS = rvs

# COMPILER COMMANDS
COMPILE_OBJ = \
	@echo "CC [${@:.o=.cpp}]"; \
	$(CC) $(CFLAGS) -c ${@:.o=.cpp} -o $@

MAKE_TEST = \
	@echo "TEST [$@]"; \
	$(CC) $(CFLAGS) $@.o -o $(addprefix $(BIN_DIR)/, $@) \
		-L. -lzp3 $(LIBS)

MAKE_EXE = \
	@echo "EXE [$@]"; \
	$(CC) $(CFLAGS) $@.o -o $(addprefix $(BIN_DIR)/, $@) \
		-L. -lzp3 $(LIBS)


# TARGETS
all: setup_dirs tests main

setup_dirs:
	mkdir -p $(BIN_DIR)

clean:
	@rm -f zp3
	@rm -f *.o

%.o: %.cpp
	$(COMPILE_OBJ)

libzp3.a: util.o music.o display.o player.o zp3.o
	$(AR) $(ARFLAGS) $@ $?

test_music: test_music.o libzp3.a
	$(MAKE_TEST)

test_player: test_player.o libzp3.a
	$(MAKE_TEST)

test_display: test_display.o libzp3.a
	$(MAKE_TEST)

tests: $(TESTS)

main: main.o libzp3.a
	$(MAKE_EXE)
