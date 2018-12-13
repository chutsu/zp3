import vlc


class PlayerState:
    PAUSED = 1
    PLAYING = 2
    SKIPPING_NEXT = 3
    SKIPPING_PREV = 4
    SKIPPING_FORWARD = 5
    SKIPPING_BACKWARDS = 6


class Player:
    def __init__(self):
        self.state = PlayerState.PAUSED
        self.play_index = 0
        self.play_queue = 0
        self.player = None

    def play(self):
        self.player = vlc.MediaPlayer("file:///path/to/track.mp3")
        self.player.play()

    def pause(self):
        self.player.pause()

    def prev(self):
        pass

    def next(self):
        pass

    def volume_up(self):
        pass

    def volume_down(self):
        pass
