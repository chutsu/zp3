# DIRS
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

MAKE_STATIC_LIB = \
	$(AR) $(ARFLAGS) $@ $?

MAKE_TEST = \
	@echo "TEST [${@:.o=}]"; \
	$(CC) $(CFLAGS) -c ${@:.o=.cpp} -o $@; \
	$(CC) $(CFLAGS) $@ \
		-o $(addprefix $(BIN_DIR)/, ${@:.o=}) \
		-L. -lzp3 $(LIBS)

MAKE_EXE = \
	@echo "EXE [$@]"; \
	$(CC) $(CFLAGS) $@.o \
		-o $(addprefix $(BIN_DIR)/, $@) \
		-L. -lzp3 $(LIBS)
