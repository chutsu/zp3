# LOAD MAKE CONFIG
include config.mk

LIB_SSD1306=deps/ssd1306/bld/libssd1306.a

default: all

all: setup_dirs libssd1306
	@make -s -C src

$(LIB_SSD1306):
	@echo "Building libssd1306 ..."
	@cd deps/ssd1306/src && make -s -f Makefile.linux

libssd1306: $(LIB_SSD1306)

libssd1306_sdl:
	@echo "Building libssd1306 ..."
	@cd deps/ssd1306/src && make -s -f Makefile.linux SDL_EMULATION=y
	@cd deps/ssd1306/examples && make -s -f Makefile.linux SDL_EMULATION=y

deps:
	@sh scripts/install_deps.sh

setup_dirs:
	@mkdir -p $(BIN_DIR)

clean:
	@rm -f src/*.o
	@rm -f bin/*
