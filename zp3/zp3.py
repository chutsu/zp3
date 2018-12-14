import os
import time
import subprocess
import threading
import unittest

import vlc
import taglib
from luma.core.interface.serial import spi
from luma.oled.device import ssd1351
# from gpiozero import Button


def file_ext(path):
    f_name, f_ext = os.path.splitext(path)
    return f_ext


def extract_files(target_dir, filters=None):
    files = []

    for file_name in os.listdir(target_dir):
        file_path = os.path.join(target_dir, file_name)

        # Check if file path is in filters
        add_file = True
        if (filters is not None) and (file_ext(file_path) not in filters):
            add_file = False

        # Add file path if file
        if os.path.isfile(file_path) and add_file:
            files.append(file_path)

        # Traverse deeper if directory
        elif os.path.isdir(file_path):
            files.extend(extract_files(file_path, filters))

    return files


class Display:
    def __init__(self):
        serial = spi(device=0, port=0)
        self.device = ssd1351(serial)

    def show_songs(self):
        pass

    def show_queue(self):
        pass

    def show_playing(self):
        pass


class Song:
    def __init__(self, file_path):
        self.file_path = file_path

        self.track_number = None
        self.track_name = None
        self.artist = None
        self.album = None
        self.date = None

        song = taglib.File(self.file_path)
        tags = song.tags
        if tags.get("TRACKNUMBER"):
            self.track_number = tags["TRACKNUMBER"][0]

        if tags.get("TITLE"):
            self.track_name = tags["TITLE"][0]

        if tags.get("ARTIST"):
            self.artist = tags["ARTIST"][0]

        if tags.get("ALBUM"):
            self.album = tags["ALBUM"][0]

        if tags.get("DATE"):
            self.date = tags["DATE"][0]

    def show(self):
        return "%s\n%s\n%s" % (self.track_name, self.artist, self.album)

    def __str__(self):
        retval = ""
        retval += "file_path: %s\n" % self.file_path
        retval += "track_number: %s\n" % self.track_number
        retval += "track_name: %s\n" % self.track_name
        retval += "album: %s\n" % self.album
        retval += "date: %s\n" % self.date
        return retval


class MusicLibrary:
    def __init__(self, music_dir):
        self.song_index = 0
        self.albums = []
        self.database = {
            "songs": [],
            "albums": {},
            "artists": {}
        }
        self.queue = []
        self._build_database(music_dir)

    def _build_database(self, music_dir):
        files = sorted(extract_files(music_dir, [".mp3", ".flac"]))
        self.queue = files

        for file_path in files:
            song = Song(file_path)

            # Add song to database
            self.database["songs"].append(song)

            # Add album to database
            if song.album not in self.database["albums"]:
                self.database["albums"][song.album] = [song]
            else:
                self.database["albums"][song.album].append(song)

            # Add artist to database
            if song.artist not in self.database["artists"]:
                self.database["artists"][song.artist] = [song]
            else:
                self.database["artists"][song.artist].append(song)

    def set_mode(self, mode, value):
        if mode == "artist":
            self.queue = self.database["artists"][value]
        elif mode == "album":
            self.queue = self.database["albums"][value]
        elif mode == "song":
            self.queue = self.database["songs"]
        else:
            self.queue = self.database["songs"]

    def get_song(self):
        return self.queue[self.song_index]

    def get_next_song(self):
        self.song_index += 1

        if self.song_index > len(self.queue):
            self.song_index = 0
        elif self.song_index < 0:
            self.song_index = len(self.queue) - 1

        return self.queue[self.song_index]

    def get_prev_song(self):
        self.song_index -= 1

        if self.song_index > len(self.queue):
            self.song_index = 0
        elif self.song_index < 0:
            self.song_index = len(self.queue) - 1

        return self.queue[self.song_index]


def song_thread(args):
    thread = threading.currentThread()
    song_path, song_time = args

    player = vlc.MediaPlayer(song_path)
    player.play()                  # Initialize player
    player.pause()                 # Pause so that we can set time
    player.set_time(song_time[0])  # Set the time
    player.play()                  # Continue playing
    player.audio_set_volume(getattr(thread, "volume"))

    # Keep thread alive
    while player.is_playing and getattr(thread, "keep_running", True):
        player.audio_set_volume(getattr(thread, "volume"))
        song_time[0] = player.get_time()

    # Stop
    player.stop()


class ZP3:
    def __init__(self, music_dir):
        self.shuffle_mode = False
        self.hold_mode = False

        self.is_playing = False
        self.song_time = [0]  # List because it has to be mutable for threads
        self.volume_level = 50
        self.volume_max = 100
        self.volume_min = 0
        self.volume_step = 10

        self.thread = None
        self.music = MusicLibrary(music_dir)
        self.display = Display()

        # self.btn_prev = Button(1)
        # self.btn_menu = Button(1)
        # self.btn_play = Button(1)
        # self.btn_next = Button(1)
        # self.btn_hold = Button(1)

        # self.btn_prev.when_pressed = self.prev
        # self.btn_menu.when_pressed = self.menu
        # self.btn_play.when_pressed = self.play
        # self.btn_next.when_pressed = self.next
        # self.btn_hold.when_held = self.hold

    def prev(self):
        if self.hold_mode:
            return

        self.music.get_prev_song()
        self._stop()
        self.play()

    def menu(self):
        if self.hold_mode:
            return

    def play(self):
        if self.hold_mode:
            return

        song_path = self.music.get_song()
        if self.is_playing is False:
            if self.song_time[0] != 0:
                song_name = os.path.basename(song_path)
                song_time = self.song_time[0] * 1e-3
                print("Resuming: [%s] at [%.2fs]" % (song_name, song_time))
            else:
                print("Playing: [%s]" % os.path.basename(song_path))
            args = [song_path, self.song_time]
            self.thread = threading.Thread(target=song_thread, args=(args,))
            self.thread.keep_running = True
            self.thread.volume = self.volume_level
            self.thread.start()
            self.is_playing = True

        elif self.is_playing is True:
            print("Pausing at [%.2fs]" % (self.song_time[0] * 1e-3))
            self.thread.keep_running = False
            self.thread.volume = self.volume_level
            self.thread.join()
            self.thread = None
            self.is_playing = False

    def _stop(self):
        if self.thread is not None:
            self.thread.keep_running = False
            self.thread.join()
            self.thread = None
            self.is_playing = False
            self.song_time = [0]

    def next(self):
        if self.hold_mode:
            return

        self.music.get_next_song()
        self._stop()
        self.play()

    def _keep_volumn_within_bounds(self):
        if self.volume_level > self.volume_max:
            self.volume_level = self.volume_max
        if self.volume_level < self.volume_min:
            self.volume_level = self.volume_min

    def volume_up(self):
        if self.hold_mode:
            return
        self.volume_level += self.volume_step
        self._keep_volumn_within_bounds()
        self.thread.volume = self.volume_level
        print("Volume increased to [%d]" % self.volume_level)

    def volume_down(self):
        if self.hold_mode:
            return
        self.volume_level -= self.volume_step
        self._keep_volumn_within_bounds()
        self.thread.volume = self.volume_level
        print("Volume decreased to [%d]" % self.volume_level)

    def hold(self):
        self.hold_mode = True if self.hold_mode is False else False

    def power_off(self):
        subprocess.call(['shutdown', '-h', 'now'], shell=False)


#############################################################################
# UNITTESTS
#############################################################################

class TestSong(unittest.TestCase):
    def test_constructor(self):
        song_path = os.path.join("test_data", "album1", "1-apple.mp3")
        song = Song(song_path)

        self.assertEqual(song_path, song.file_path)
        self.assertEqual("Apple", song.track_name)
        self.assertEqual("1", song.track_number)
        self.assertEqual("Bob Dylan", song.artist)
        self.assertEqual("ALBUM1", song.album)
        self.assertEqual("2018", song.date)

    def test_show(self):
        song_path = os.path.join("test_data", "album1", "1-apple.mp3")
        song = Song(song_path)
        song.show()


class TestMusicLibrary(unittest.TestCase):
    def setUp(self):
        self.music = MusicLibrary("test_data")
        # self.music = MusicLibrary("/data/music")
        self.assertTrue(len(self.music.queue) > 0)

    def test_dummy(self):
        pass

    def test_get_song(self):
        song = self.music.get_song()
        self.assertEqual("test_data/album1/1-apple.mp3", song)
        self.assertEqual(0, self.music.song_index)

    def test_get_prev_song(self):
        song = self.music.get_prev_song()
        self.assertEqual("test_data/album2/5-foxtrot.mp3", song)
        self.assertEqual(9, self.music.song_index)

    def test_get_next_song(self):
        song = self.music.get_next_song()
        self.assertEqual("test_data/album1/2-banana.mp3", song)
        self.assertEqual(1, self.music.song_index)


class TestZP3(unittest.TestCase):
    def setUp(self):
        self.zp3 = ZP3("/data/music/")

    def test_play(self):
        # Play song
        self.zp3.play()
        self.assertTrue(self.zp3.is_playing)

        # Sleep for 5 seconds
        time.sleep(5)

        # Pause for 3 seocnds
        self.zp3.play()
        self.assertFalse(self.zp3.is_playing)
        time.sleep(3)

        # Resume playing for another 10 seconds
        self.zp3.play()
        time.sleep(10)

        # Stop
        self.zp3._stop()

    def test_next(self):
        # Play song
        self.zp3.play()
        self.assertTrue(self.zp3.is_playing)

        # Sleep for 5 seconds
        time.sleep(5)

        # Play next
        self.zp3.next()
        time.sleep(10)
        self.zp3._stop()

    def test_prev(self):
        # Play song
        self.zp3.play()
        self.assertTrue(self.zp3.is_playing)

        # Sleep for 5 seconds
        time.sleep(5)

        # Play next
        self.zp3.prev()
        time.sleep(10)
        self.zp3._stop()

    def test_volume_up(self):
        # Play song
        self.zp3.play()
        self.assertTrue(self.zp3.is_playing)

        # Sleep for 2 seconds
        time.sleep(2)

        # Volume up
        self.zp3.volume_up()
        self.zp3.volume_up()
        self.zp3.volume_up()
        self.zp3.volume_up()

        # Sleep for 5 seconds
        time.sleep(5)
        self.zp3._stop()

    def test_volume_down(self):
        # Play song
        self.zp3.play()
        self.assertTrue(self.zp3.is_playing)

        # Sleep for 2 seconds
        time.sleep(2)

        # Volume up
        self.zp3.volume_down()
        self.zp3.volume_down()
        self.zp3.volume_down()
        self.zp3.volume_down()

        # Sleep for 5 seconds
        time.sleep(5)
        self.zp3._stop()
