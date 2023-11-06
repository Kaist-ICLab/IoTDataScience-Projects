import pygame

import serial
import random
import time
import threading
import csv


last_mode="using"
current_mode="using"
last_music_mode="using"
current_mode_counter=0


# Define the labels
usage_mode = ["away", "using", "stressed"]


# Example usage
songs={"using":"lofi.mp3","stressed":"peacefull.mp3"}


def play_music(file_path):
    pygame.mixer.init()
    pygame.mixer.music.load(file_path)
    pygame.mixer.music.play()

def stop_music():
    pygame.mixer.music.stop()

def pause_music():
    pygame.mixer.music.pause()

def resume_music():
    pygame.mixer.music.unpause()


#stop_music()


# After a few seconds, resume the music
#resume_music()




def read_arduino_usage_mode(comport, baudrate):
    ser = serial.Serial(comport, baudrate, timeout=0.1)
    global last_mode 
    global current_mode 
    global current_mode_counter
    global last_music_mode

    while True:
        data = ser.readline().decode().strip()
        if data:
            print(data)
            data_tab=data.split(":")
            if len(data_tab)==2:
                current_mode=data_tab[1]
    
            if current_mode=="away":
                if current_mode_counter>10:
                    stop_music()
                    last_music_mode="away"
            elif current_mode!=last_mode :
                current_mode_counter=0
                
                if current_mode!=last_music_mode:
                    stop_music()
                    play_music(songs[current_mode])
                    last_music_mode=current_mode
            last_mode=current_mode
            current_mode_counter+=1




if __name__ == '__main__':
    comport = 'COM3'
    baudrate = 9600

    play_music(songs[current_mode])

    serial_thread = threading.Thread(target=read_arduino_usage_mode, args=(comport, baudrate))
    serial_thread.daemon=True
    #index_thread = threading.Thread(target=change_index)
    #index_thread.daemomn=True

    #index_thread.start()
    serial_thread.start()

    while(True):
        pass
  
    
    