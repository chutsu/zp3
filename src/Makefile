# LOAD MAKE CONFIG
include $(PWD)/config.mk

# FILES AND DIRS
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
TESTS = test_music.o test_player.o test_display.o

# TARGETS
default: $(TESTS) main

%.o: %.cpp
	$(COMPILE_OBJ)

test_%.o: test_%.cpp libzp3.a
	$(MAKE_TEST)

libzp3.a: util.o gpio.o music.o display.o player.o zp3.o
	$(MAKE_STATIC_LIB)

main: main.o libzp3.a
	$(MAKE_EXE)
