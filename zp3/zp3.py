import vlc
from gpiozero import Button


class Display:
    def __init__(self):
        pass

    def show_songs(self):
        pass

    def show_queue(self):
        pass

    def show_playing(self):
        pass


class Playlist:
    def __init__(self, music_dir):
        self.song_index = 0
        self.database = []
        self.queue = []

    def get_song(self):
        pass

    def get_next_song(self):
        pass

    def get_prev_song(self):
        pass


class ZP3:
    def __init__(self, music_dir):
        self.is_playing = False
        self.shuffle_mode = False
        self.hold_mode = False
        self.volume_level = 0.3

        self.player = vlc.MediaPlayer()
        self.playlist = Playlist(music_dir)
        self.display = Display()

        self.btn_prev = Button(1)
        self.btn_menu = Button(1)
        self.btn_play = Button(1)
        self.btn_next = Button(1)
        self.btn_hold = Button(1)

        self.btn_prev.when_pressed = self.prev
        self.btn_menu.when_pressed = self.menu
        self.btn_play.when_pressed = self.play
        self.btn_next.when_pressed = self.next
        self.btn_hold.when_held = self.hold

    def prev(self):
        if self.hold_mode:
            return

    def menu(self):
        if self.hold_mode:
            return

    def play(self):
        if self.hold_mode:
            return

        if self.is_playing is False:
            self.player.play()
            self.is_playing = True
        elif self.is_playing is True:
            self.player.pause()
            self.is_playing = False

    def next(self):
        if self.hold_mode:
            return

    def volume_up(self):
        if self.hold_mode:
            return

        if self.volume_level >= 1.0:
            self.volume_level = 1.0
        else:
            self.volume_level += 0.1

    def volume_down(self):
        if self.hold_mode:
            return

        if self.volume_level <= 0.0:
            self.volume_level = 0.0
        else:
            self.volume_level -= 0.1

    def hold(self):
        if self.hold_mode is False:
            self.hold_mode = True
        else:
            self.hold_mode = False
