all: deps tests

deps:
	@sh scripts/install_deps.sh

tests:
	@cd zp3 && python3 -m unittest -v -b zp3.py

test_display:
	@cd zp3 && python3 -m unittest -v zp3.TestDisplay

test_song:
	@cd zp3 && python3 -m unittest -v -b zp3.TestSong

test_music_library:
	@cd zp3 && python3 -m unittest -v -b zp3.TestMusicLibrary

test_zp3:
	@cd zp3 && python3 -m unittest -v -b zp3.TestZP3
