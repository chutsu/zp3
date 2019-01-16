import os
import time
import subprocess
import threading
import unittest

import vlc
import mutagen
import mutagen.flac
import mutagen.mp3
#import gpiozero as gpio
from PIL import ImageFont
from luma.core.interface.serial import spi
from luma.oled.device import ssd1351
from luma.core.render import canvas


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


class Song:
    def __init__(self, file_path):
        self.file_path = file_path

        self.track_number = None
        self.track_name = None
        self.length = None
        self.artist = None
        self.album = None
        self.date = None

        if file_ext(file_path) == ".mp3":
            self.length = mutagen.mp3.MP3(file_path).info.length
        if file_ext(file_path) == ".flac":
            self.length = mutagen.flac.FLAC(file_path).info.length

        tags = mutagen.File(self.file_path)
        if tags.get("tracknumber"):
            self.track_number = tags["tracknumber"][0]

        if tags.get("title"):
            self.track_name = tags["title"][0]

        if tags.get("artist"):
            self.artist = tags["artist"][0]

        if tags.get("album"):
            self.album = tags["album"][0]

        if tags.get("date"):
            self.date = tags["date"][0]

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

        if self.song_index >= len(self.queue):
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


class Display:
    def __init__(self):
        serial = spi(device=0, port=0)
        self.device = ssd1351(serial)
        self.font_path = "/usr/share/fonts/truetype/msttcorefonts/verdana.ttf"
        self.font_size = 10
        self.font = ImageFont.truetype(self.font_path, self.font_size)

        self.track_scroll_counter = 0
        self.track_scroll_pause = 0
        self.track_pause_threshold = 50
        self.menu_item_scroll_counter = 0
        self.menu_item_scroll_pause = 0

    def reset_counters(self):
        self.track_scroll_counter = 0
        self.track_scroll_pause = 0
        self.menu_item_scroll_counter = 0
        self.menu_item_scroll_pause = 0

    def show_loading(self):
        with canvas(self.device) as draw:
            x = 20
            y = 60
            text = "LOADING ..."
            fill = "white"
            font_size = 15
            font = ImageFont.truetype(self.font_path, font_size)
            draw.text((x, y), text, fill=fill, font=font)

    def show_menu(self, songs, index):
        with canvas(self.device) as draw:
            height_step = (self.device.height / 10)
            height_padding = 0
            width_padding = 2

            menu_page = int(index / 10.0)
            song_index = index % 10
            song_list = list(songs)
            song_list = song_list[10 * menu_page:]

            for i in range(len(song_list)):
                # Selected
                top_left = (0, height_step * i)
                bottom_right = (self.device.width, height_step * (i + 1))
                rect_cord = [top_left, bottom_right]
                fill = "white" if i == song_index else "black"
                draw.rectangle(rect_cord, fill=fill)

                # Text
                x = width_padding - self.menu_item_scroll_counter
                y = (height_step * i) + height_padding
                text = "%s" % song_list[i]
                fill = "black" if i == song_index else "white"
                draw.text((x, y), text, fill=fill, font=self.font)

                # Update scroll
                if i == song_index and len(text) > 17:
                    if self.menu_item_scroll_pause < 15:
                        self.menu_item_scroll_pause += 1
                    else:
                        self.menu_item_scroll_counter += 5
                        end = (len(text) * 7) - self.menu_item_scroll_counter
                        if end < 0:
                            self.menu_item_scroll_counter = 0
                            self.menu_item_scroll_pause = 0

    def _track_name(self, draw, track_name):
        x = 5 - self.track_scroll_counter
        y = 26
        text = track_name
        fill = "white"
        font_size = 11
        font = ImageFont.truetype(self.font_path, font_size)
        draw.text((x, y), text, fill=fill, font=font)

    def _track_artist(self, draw, track_artist):
        x = 5 - self.track_scroll_counter
        y = 45
        text = track_artist
        fill = "white"
        draw.text((x, y), text, fill=fill, font=self.font)

    def _track_album(self, draw, track_album):
        x = 5 - self.track_scroll_counter
        y = 60
        text = track_album
        fill = "white"
        draw.text((x, y), text, fill=fill, font=self.font)

    def _track_progress(self, draw, track_progress):
        # Track progress
        # -- Progress outline
        top_left = (10, 80)
        bottom_right = (self.device.width - 10, top_left[1] + 10)
        rect_cord = [top_left, bottom_right]
        draw.rectangle(rect_cord, outline="white", fill="black")
        # -- Progress bar
        progress_bar_width = (self.device.width - 10) - 10
        bottom_right_x = 10 + (progress_bar_width * track_progress)
        bottom_right = (bottom_right_x, top_left[1] + 10)
        rect_cord = [top_left, bottom_right]
        draw.rectangle(rect_cord, fill="white")

    def _track_status(self, draw, status):
        if status == "PAUSE":
            # Pause box
            width = 14
            center = (self.device.width / 2.0, 110)
            top_left = (center[0] - width / 2.0, center[1] - width / 2.0)
            bottom_right = (center[0] + width / 2.0, center[1] + width / 2.0)
            rect_cord = [top_left, bottom_right]
            fill = "white"
            draw.rectangle(rect_cord, fill=fill)

            # Pause split
            height = 14
            width = 4
            center = (self.device.width / 2.0, 110)
            top_left = (center[0] - width / 2.0, center[1] - height / 2.0)
            bottom_right = (center[0] + width / 2.0, center[1] + height / 2.0)
            rect_cord = [top_left, bottom_right]
            fill = "black"
            draw.rectangle(rect_cord, fill=fill)

        elif status == "PLAY":
            width = 14
            center = (self.device.width / 2.0, 110)
            top_left = (center[0] - width / 2.0, center[1] - width / 2.0)
            bottom_left = (center[0] - width / 2.0, center[1] + width / 2.0)
            middle_right = (center[0] + width / 2.0, center[1])
            poly_cords = [top_left, bottom_left, middle_right]
            fill = "white"
            draw.polygon(poly_cords, fill=fill)

    def show_playing(self, song, song_time, status):
        track_name = song.track_name
        track_artist = song.artist
        track_album = song.album
        song_length = song.length
        progress = (song_time * 1e-3) / song_length
        progress = 0.0 if progress < 0.0 else progress
        progress = 1.0 if progress > 1.0 else progress

        max_len = max(len(track_name), len(track_artist), len(track_album))
        if max_len > 18:
            if self.track_scroll_pause < self.track_pause_threshold:
                self.track_scroll_pause += 1
            else:
                self.track_scroll_counter += 6
                end = (max_len * 7) - self.track_scroll_counter
                if end < 0:
                    self.track_scroll_pause = 0
                    self.track_scroll_counter = 0

        with canvas(self.device) as draw:
            self._track_artist(draw, track_artist)
            self._track_album(draw, track_album)
            self._track_name(draw, track_name)
            self._track_progress(draw, progress)
            self._track_status(draw, status)

    def show_volume(self, percentage):
        width = 5
        with canvas(self.device) as draw:
            # -- Volume text
            x = 38
            y = 40
            text = "Volume"
            fill = "white"
            font_size = 15
            font = ImageFont.truetype(self.font_path, font_size)
            draw.text((x, y), text, fill=fill, font=font)

            # -- Volume empty boxes
            for i in range(0, 20, 2):
                top_left = (15 + (width * i), 70)
                bottom_right = (15 + (width * (i + 1)), 80)
                rect_cord = [top_left, bottom_right]
                fill = "black"
                outline = "white"
                draw.rectangle(rect_cord, outline=outline, fill=fill)

            # -- Volume full boxes
            for i in range(0, int(20 * percentage), 2):
                top_left = (15 + (width * i), 70)
                bottom_right = (15 + (width * (i + 1)), 80)
                rect_cord = [top_left, bottom_right]
                fill = "white"
                outline = "white"
                draw.rectangle(rect_cord, outline=outline, fill=fill)

    def show_power(self, status):
        with canvas(self.device) as draw:
            x = 7
            y = 60
            text = "POWERING ON" if status else "POWERING OFF"
            fill = "white"
            font_size = 15
            font = ImageFont.truetype(self.font_path, font_size)
            draw.text((x, y), text, fill=fill, font=font)


def song_thread(args):
    thread = threading.currentThread()
    song_path, song_time, display, callback = args

    player = vlc.MediaPlayer(song_path)
    player.play()                  # Initialize player
    player.pause()                 # Pause so that we can set time
    player.set_time(song_time[0])  # Set the time
    player.play()                  # Continue playing
    player.audio_set_volume(int(getattr(thread, "volume")))

    # Keep thread alive
    song_finished = False
    while player.is_playing:
        volume = int(getattr(thread, "volume"))
        keep_running = getattr(thread, "keep_running")
        is_player_mode = getattr(thread, "is_player_mode")

        # Update volume
        player.audio_set_volume(volume)

        # Song time is weird. player.get_time() does not immediately return the
        # the correct time, therefore must check if larger than 0.
        if player.get_time() > 0:
            song_time[0] = player.get_time()
            if is_player_mode:
                display.show_playing(Song(song_path), song_time[0], "PLAY")

        # Song_time never actually reaches the end of the song
        # so we have to hack it a little bit.
        song_length = Song(song_path).length
        if (song_length - (song_time[0] * 1e-3)) < 0.3:
            song_finished = True
            break

        # Check if we should still playing
        if keep_running is False and is_player_mode:
            display.show_playing(Song(song_path), song_time[0], "PAUSE")
            break

    # Clean up
    time.sleep(1)
    player.stop()
    if song_finished:
        callback(False)


class ZP3:
    def __init__(self, music_dir):
        self.shuffle_mode = False
        self.hold_mode = False

        self.is_playing = False
        self.song_time = [0]  # List because it has to be mutable for threads
        self.volume_level = 50.0
        self.volume_max = 100.0
        self.volume_min = 0.0
        self.volume_step = 10.0
        self.menu_index = 0

        self.thread = None
        self.music = MusicLibrary(music_dir)
        self.songs = [Song(s).track_name for s in self.music.queue]
        self.display = Display()

        self.btn_up = gpio.Button(5)
        self.btn_down = gpio.Button(6)
        self.btn_left = gpio.Button(13)
        self.btn_right = gpio.Button(19)
        self.btn_vol_up = gpio.Button(2, bounce_time=1)
        self.btn_vol_down = gpio.Button(4, bounce_time=1)

        self.is_player_mode = False
        self.menu_mode()

    def menu_mode(self):
        # Stop player if playing
        self.is_player_mode = False

        # Get latest song index
        self.menu_index = self.music.song_index

        # Show menu
        self.display.reset_counters()
        self.display.show_menu(self.songs, self.menu_index)

        # Listen for menu events
        while True:
            if self.btn_up.is_pressed:
                self.menu_index -= 1
                self._keep_menu_index_within_bounds()
                self.display.reset_counters()

            if self.btn_down.is_pressed:
                self.menu_index += 1
                self._keep_menu_index_within_bounds()
                self.display.reset_counters()

            if self.btn_left.is_pressed:
                pass

            if self.btn_right.is_pressed:
                self.music.song_index = self.menu_index
                self.display.show_loading()
                self.display.reset_counters()
                self.player_mode()
                return

            self.display.show_menu(self.songs, self.menu_index)

    def player_mode(self):
        # Check if we are in player mode
        self.is_player_mode = True
        if self.thread:
            self.thread.is_player_mode = self.is_player_mode
        self.play()

        # Register event handlers
        self.btn_up.when_pressed = self.prev
        self.btn_down.when_pressed = self.next
        self.btn_left.when_pressed = None
        self.btn_right.when_pressed = self.play
        self.btn_vol_up.when_pressed = self.volume_mode
        self.btn_vol_down.when_pressed = self.volume_mode

        while self.btn_left.is_pressed is False:
            pass

        # Register event handlers
        self.btn_up.when_pressed = None
        self.btn_down.when_pressed = None
        self.btn_left.when_pressed = None
        self.btn_right.when_pressed = None
        self.btn_vol_up.when_pressed = None
        self.btn_vol_down.when_pressed = None

        # Go to menu
        self._stop(True)
        self.menu_mode()

    def _keep_menu_index_within_bounds(self):
        if self.menu_index < 0:
            self.menu_index = 0
        elif (self.menu_index + 1) >= len(self.songs):
            self.menu_index = len(self.songs) - 1

    def play(self):
        song_path = self.music.get_song()
        if self.is_playing is False:
            if self.song_time[0] != 0:
                song_name = os.path.basename(song_path)
                song_time = self.song_time[0] * 1e-3
                print("Resuming: [%s] at [%.2fs]" % (song_name, song_time))
            else:
                print("Playing: [%s]" % os.path.basename(song_path))
            self.display.show_playing(Song(song_path), self.song_time[0], "PLAY")
            args = [song_path, self.song_time, self.display, self.next]
            self.thread = threading.Thread(target=song_thread, args=(args,))
            self.thread.daemon = True
            self.thread.keep_running = True
            self.thread.is_player_mode = self.is_player_mode
            self.thread.volume = self.volume_level
            self.thread.start()
            self.is_playing = True

        elif self.is_playing is True:
            print("Pausing at [%.2fs]" % (self.song_time[0] * 1e-3))
            song_path = self.music.get_song()
            self.thread.keep_running = False
            self.thread.volume = self.volume_level
            self.thread.join()
            self.thread = None
            self.is_playing = False
            self.display.show_playing(Song(song_path), self.song_time[0], "PAUSE")

    def _stop(self, wait_for_join=True):
        if self.thread is not None:
            self.thread.keep_running = False
            if wait_for_join:
                self.thread.join()

        self.thread = None
        self.is_playing = False
        self.song_time = [0]

    def prev(self):
        self._stop()
        self.display.show_loading()
        self.music.get_prev_song()
        self.display.reset_counters()
        self.play()

    def next(self, wait_for_join=True):
        self._stop(wait_for_join)
        self.display.show_loading()
        self.music.get_next_song()
        self.display.reset_counters()
        self.play()

    def _keep_volumn_within_bounds(self):
        if self.volume_level > self.volume_max:
            self.volume_level = self.volume_max
        if self.volume_level < self.volume_min:
            self.volume_level = self.volume_min

    def volume_up(self):
        self.volume_level += self.volume_step
        self._keep_volumn_within_bounds()
        print("Volume increased to [%d]" % self.volume_level)

    def volume_down(self):
        self.volume_level -= self.volume_step
        self._keep_volumn_within_bounds()
        print("Volume decreased to [%d]" % self.volume_level)

    def volume_mode(self):
        self.thread.is_player_mode = False
        time.sleep(0.5)
        self.btn_vol_up.when_pressed = None
        self.btn_vol_down.when_pressed = None

        self.display.show_volume(self.volume_level / self.volume_max)
        time_start = time.time()
        while True:
            if self.btn_vol_up.is_pressed:
                self.volume_level += self.volume_step
                self._keep_volumn_within_bounds()
                print("Volume increased to [%d]" % self.volume_level)
                time_start = time.time()
            elif self.btn_vol_down.is_pressed:
                self.volume_level -= self.volume_step
                print("Volume decreased to [%d]" % self.volume_level)
                time_start = time.time()
            elif (time.time() - time_start) > 3.0:
                break
            time.sleep(0.05)

            self.thread.volume = self.volume_level
            self._keep_volumn_within_bounds()
            self.display.show_volume(self.volume_level / self.volume_max)

        self.thread.is_player_mode = True
        self.btn_vol_up.when_pressed = self.volume_mode
        self.btn_vol_down.when_pressed = self.volume_mode


#############################################################################
# UNITTESTS
#############################################################################

def _btn_up_handler():
    print("Up button pressed!")

def _btn_down_handler():
    print("Down button pressed!")

def _btn_left_handler():
    print("Left button pressed!")

def _btn_right_handler():
    print("Right button pressed!")

def _btn_enter_handler():
    print("Enter button pressed!")


class TestButtons(unittest.TestCase):
    def test_sandbox(self):
        btn_up = gpio.Button(5)
        btn_down = gpio.Button(6)
        btn_left = gpio.Button(13)
        btn_right = gpio.Button(19)
        btn_enter = gpio.Button(26)

        btn_up.when_pressed = _btn_up_handler
        btn_down.when_pressed = _btn_down_handler
        btn_left.when_pressed = _btn_left_handler
        btn_right.when_pressed = _btn_right_handler
        btn_enter.when_pressed = _btn_enter_handler

        import signal
        signal.pause()


class TestDisplay(unittest.TestCase):
    def test_show_menu(self):
        display = Display()
        songs = range(10)
        menu_index = 5
        display.show_menu(songs, menu_index)
        time.sleep(5)

    def test_show_playing(self):
        song_path = "/home/pi/music/The Killers - Hot Fuss (2004)/01 - Jenny Was A Friend Of Mine.flac"
        song = Song(song_path)

        display = Display()
        while True:
            display.show_playing(song, 120, "PLAY")
            time.sleep(0.05)
        time.sleep(5)

    def test_show_loading(self):
        display = Display()
        display.show_loading()
        time.sleep(5)

    def test_show_volume(self):
        display = Display()
        display.show_volume(0.4)
        time.sleep(5)


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
    #def setUp(self):
    #    self.zp3 = ZP3("/home/pi/music/")

    # def test_play(self):
    #     # Play song
    #     self.zp3.play()
    #     self.assertTrue(self.zp3.is_playing)

    #     # Sleep for 5 seconds
    #     time.sleep(5)

    #     # Pause for 3 seocnds
    #     self.zp3.play()
    #     self.assertFalse(self.zp3.is_playing)
    #     time.sleep(3)

    #     # Resume playing for another 10 seconds
    #     self.zp3.play()
    #     time.sleep(10)

    #     # Stop
    #     self.zp3._stop()

    # def test_next(self):
    #     # Play song
    #     self.zp3.play()
    #     self.assertTrue(self.zp3.is_playing)

    #     # Sleep for 5 seconds
    #     time.sleep(5)

    #     # Play next
    #     self.zp3.next()
    #     time.sleep(10)
    #     self.zp3._stop()

    # def test_prev(self):
    #     # Play song
    #     self.zp3.play()
    #     self.assertTrue(self.zp3.is_playing)

    #     # Sleep for 5 seconds
    #     time.sleep(5)

    #     # Play next
    #     self.zp3.prev()
    #     time.sleep(10)
    #     self.zp3._stop()

    # def test_volume_up(self):
    #     # Play song
    #     self.zp3.play()
    #     self.assertTrue(self.zp3.is_playing)

    #     # Sleep for 2 seconds
    #     time.sleep(2)

    #     # Volume up
    #     self.zp3.volume_up()
    #     self.zp3.volume_up()
    #     self.zp3.volume_up()
    #     self.zp3.volume_up()

    #     # Sleep for 5 seconds
    #     time.sleep(5)
    #     self.zp3._stop()

    # def test_volume_down(self):
    #     # Play song
    #     self.zp3.play()
    #     self.assertTrue(self.zp3.is_playing)

    #     # Sleep for 2 seconds
    #     time.sleep(2)

    #     # Volume up
    #     self.zp3.volume_down()
    #     self.zp3.volume_down()
    #     self.zp3.volume_down()
    #     self.zp3.volume_down()

    #     # Sleep for 5 seconds
    #     time.sleep(5)
    #     self.zp3._stop()

    def test_loop(self):
        zp3 = ZP3("/home/pi/music/")
        zp3.menu_mode()
        import signal
        signal.pause()
