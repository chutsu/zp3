set -e

# echo $PWD
# cd zp3
# python3 -m unittest zp3.py

# python deps/ssd1306/tools/fontgenerator.py \
#   --ttf scripts/FreeMono.ttf \
#   -s 6 \
#   -f old > src/font_freemono.hpp

# make tests
# make deps
# make test_buttons
# make test_display
# make test_song
# make test_music_library
# make test_zp3

# cd deps/ssd1306/src \
#   && make -f Makefile.linux SDL_EMULATION=y all \
#   cd -

# make clean
make
# ./bin/test_music
./bin/test_player
# ./bin/test_display

# cd src \
#   && make \
#   && ./zp3
