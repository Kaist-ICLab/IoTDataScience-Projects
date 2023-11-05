import subprocess

def send_dbus_message(arg):
     subprocess.run(["dbus-send", "--print-reply", "--dest=org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", arg])

def play_pause():
    send_dbus_message('org.mpris.MediaPlayer2.Player.PlayPause')

def next_track():
    send_dbus_message('org.mpris.MediaPlayer2.Player.Next')

def previous_track():
    send_dbus_message('org.mpris.MediaPlayer2.Player.Previous')
