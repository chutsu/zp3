import os
import taglib

os.system("mkdir -p album1 album2 album3")
os.system("touch 'album1/1-apple.mp3'")
os.system("touch 'album1/2-banana.mp3'")
os.system("touch 'album1/3-cherries.mp3'")
os.system("touch 'album1/4-dates.mp3'")
os.system("touch 'album1/5-figs.mp3'")

os.system("touch 'album2/1-alpha.mp3'")
os.system("touch 'album2/2-bravo.mp3'")
os.system("touch 'album2/3-charlie.mp3'")
os.system("touch 'album2/4-delta.mp3'")
os.system("touch 'album2/5-foxtrot.mp3'")

os.system("touch 'album3/1-track1.mp3'")
os.system("touch 'album3/2-track2.mp3'")
os.system("touch 'album3/3-track3.mp3'")
os.system("touch 'album3/4-track4.mp3'")
os.system("touch 'album3/5-track5.mp3'")


song = taglib.File("album1/1-apple.mp3")
song.tags["ALBUM"] = ["ALBUM1"]
song.tags["ARTIST"] = ["Bob Dylan"]
song.tags["TITLE"] = ["Apple"]
song.tags["TRACKNUMBER"] = ["1"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album1/2-banana.mp3")
song.tags["ALBUM"] = ["ALBUM1"]
song.tags["ARTIST"] = ["Bob Dylan"]
song.tags["TITLE"] = ["Banana"]
song.tags["TRACKNUMBER"] = ["2"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album1/3-cherries.mp3")
song.tags["ALBUM"] = ["ALBUM1"]
song.tags["ARTIST"] = ["Bob Dylan"]
song.tags["TITLE"] = ["Cherries"]
song.tags["TRACKNUMBER"] = ["3"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album1/4-dates.mp3")
song.tags["ALBUM"] = ["ALBUM1"]
song.tags["ARTIST"] = ["Bob Dylan"]
song.tags["TITLE"] = ["Dates"]
song.tags["TRACKNUMBER"] = ["4"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album1/5-figs.mp3")
song.tags["ALBUM"] = ["ALBUM1"]
song.tags["ARTIST"] = ["Bob Dylan"]
song.tags["TITLE"] = ["Figs"]
song.tags["TRACKNUMBER"] = ["5"]
song.tags["DATE"] = ["2018"]
song.save()


song = taglib.File("album2/1-alpha.mp3")
song.tags["ALBUM"] = ["ALBUM2"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Alpha"]
song.tags["TRACKNUMBER"] = ["1"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album2/2-bravo.mp3")
song.tags["ALBUM"] = ["ALBUM2"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Bravo"]
song.tags["TRACKNUMBER"] = ["2"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album2/3-charlie.mp3")
song.tags["ALBUM"] = ["ALBUM2"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Charlie"]
song.tags["TRACKNUMBER"] = ["3"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album2/4-delta.mp3")
song.tags["ALBUM"] = ["ALBUM2"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Delta"]
song.tags["TRACKNUMBER"] = ["4"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album2/5-foxtrot.mp3")
song.tags["ALBUM"] = ["ALBUM2"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Foxtrot"]
song.tags["TRACKNUMBER"] = ["5"]
song.tags["DATE"] = ["2018"]
song.save()


song = taglib.File("album3/1-track1.mp3")
song.tags["ALBUM"] = ["ALBUM3"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Track1"]
song.tags["TRACKNUMBER"] = ["1"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album3/2-track2.mp3")
song.tags["ALBUM"] = ["ALBUM3"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Track2"]
song.tags["TRACKNUMBER"] = ["2"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album3/3-track3.mp3")
song.tags["ALBUM"] = ["ALBUM3"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Track3"]
song.tags["TRACKNUMBER"] = ["3"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album3/4-track4.mp3")
song.tags["ALBUM"] = ["ALBUM3"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Track4"]
song.tags["TRACKNUMBER"] = ["4"]
song.tags["DATE"] = ["2018"]
song.save()

song = taglib.File("album3/5-track5.mp3")
song.tags["ALBUM"] = ["ALBUM3"]
song.tags["ARTIST"] = ["Michael Jackson"]
song.tags["TITLE"] = ["Track5"]
song.tags["TRACKNUMBER"] = ["5"]
song.tags["DATE"] = ["2018"]
song.save()
