import time
from os import listdir
from os.path import isfile
from os.path import isdir
from os.path import join
from os.path import basename
from os.path import splitext
import threading
from threading import Thread
import unittest

import vlc
# from gpiozero import Button


def file_ext(path):
    f_name, f_ext = splitext(path)
    return f_ext


def extract_files(target_dir, filters=None):
    files = []

    for file_name in listdir(target_dir):
        file_path = join(target_dir, file_name)

        # Check if file path is in filters
        add_file = True
        if (filters is not None) and (file_ext(file_path) not in filters):
            add_file = False

        # Add file path if file
        if isfile(file_path) and add_file:
            files.append(file_path)

        # Traverse deeper if directory
        elif isdir(file_path):
            files.extend(extract_files(file_path, filters))

    return files


class Display:
    def __init__(self):
        pass

    def show_songs(self):
        pass

    def show_queue(self):
        pass

    def show_playing(self):
        pass


class MusicLibrary:
    def __init__(self, music_dir):
        self.song_index = 0
        self.database = sorted(extract_files(music_dir, [".mp3", ".flac"]))

    def get_song(self):
        return self.database[self.song_index]

    def get_next_song(self):
        self.song_index += 1

        if self.song_index > len(self.database):
            self.song_index = 0
        elif self.song_index < 0:
            self.song_index = len(self.database) - 1

        return self.database[self.song_index]

    def get_prev_song(self):
        self.song_index -= 1

        if self.song_index > len(self.database):
            self.song_index = 0
        elif self.song_index < 0:
            self.song_index = len(self.database) - 1

        return self.database[self.song_index]


def song_thread(args):
    thread = threading.currentThread()
    song_path, song_time = args

    player = vlc.MediaPlayer(song_path)
    player.play()                  # Initialize player
    player.pause()                 # Pause so that we can set time
    player.set_time(song_time[0])  # Set the time
    player.play()                  # Continue playing

    # Keep thread alive
    while player.is_playing and getattr(thread, "keep_running", True):
        song_time[0] = player.get_time()

    # Stop
    player.stop()


class ZP3:
    def __init__(self, music_dir):
        self.shuffle_mode = False
        self.hold_mode = False

        self.is_playing = False
        self.song_time = [0]  # List because it has to be mutable for threads
        self.volume_level = 0.3

        self.player_thread = None
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
                song_name = basename(song_path)
                song_time = self.song_time[0] * 1e-3
                print("Resuming: [%s] at [%.2fs]" % (song_name, song_time))
            else:
                print("Playing: [%s]" % basename(song_path))
            args = [song_path, self.song_time]
            self.player_thread = Thread(target=song_thread, args=(args,))
            self.player_thread.start()
            self.is_playing = True

        elif self.is_playing is True:
            print("Pausing at [%.2fs]" % (self.song_time[0] * 1e-3))
            self.player_thread.keep_running = False
            self.player_thread.join()
            self.player_thread = None
            self.is_playing = False

    def _stop(self):
        if self.player_thread is not None:
            self.player_thread.keep_running = False
            self.player_thread.join()
            self.player_thread = None
            self.is_playing = False
            self.song_time = [0]

    def next(self):
        if self.hold_mode:
            return

        self.music.get_next_song()
        self._stop()
        self.play()

    def _keep_volumn_within_bounds(self):
        if self.volume_level > 1.0:
            self.volume_level = 1.0
        if self.volume_level < 0.0:
            self.volume_level = 0.0

    def volume_up(self):
        if self.hold_mode:
            return

        self.volume_level += 0.1
        self._keep_volumn_within_bounds()

    def volume_down(self):
        if self.hold_mode:
            return

        self.volume_level -= 0.1
        self._keep_volumn_within_bounds()

    def hold(self):
        self.hold_mode = True if self.hold_mode is False else False


#############################################################################
# UNITTESTS
#############################################################################


class TestMusicLibrary(unittest.TestCase):
    def setUp(self):
        self.music = MusicLibrary("test_data")
        self.assertEqual(10, len(self.music.database))

    def test_get_song(self):
        song = self.music.get_song()
        self.assertEqual("test_data/album1/1 - apple.mp3", song)
        self.assertEqual(0, self.music.song_index)

    def test_get_prev_song(self):
        song = self.music.get_prev_song()
        self.assertEqual("test_data/album2/5 - foxtrot.mp3", song)
        self.assertEqual(9, self.music.song_index)

    def test_get_next_song(self):
        song = self.music.get_next_song()
        self.assertEqual("test_data/album1/2 - banana.mp3", song)
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
