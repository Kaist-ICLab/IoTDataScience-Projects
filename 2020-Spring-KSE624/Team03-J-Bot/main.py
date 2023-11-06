#!/usr/bin/env python
# coding: utf-8
'''
Final Project for KSE624 Mobile and Pervasive Computing for Knowledge Services Spring 2020 at KAIST

Last Updated Date: July 01 2020
Authors: 
    Rafikatiwi Nur Pujiarti
    Willmer R. Quinones

-----------------------------

main.py

'''

## Necessary Packages
from codes.jetbot_actions import (speech_to_text, trigger_speech, text_to_wav, play)
import traitlets
import ipywidgets.widgets as widgets
from jetbot import Camera, bgr8_to_jpeg

'''
Initiating the J-Bot camera for the first time is a slow process, hence we initiate the camera
once the J-Bot is turn on
'''
camera = Camera.instance(width=224, height=224)
image = widgets.Image(format='jpeg', width=224, height=224)
camera_link = traitlets.dlink((camera, 'value'), (image, 'value'), transform=bgr8_to_jpeg)
camera.stop()

count = 0
session = True
while session:
    # Waiting for the user to speak to J-Bot
    keyword = speech_to_text()


    # J-Bot wakes up if the user greet her
    if (count == 0) and (keyword == 'hello robot'): 
        count = 1
        response_sentence = trigger_speech('greeting')
        play(text_to_wav('en-US-Wavenet-F', response_sentence))

    # If the user ask J-Bot for the weather, she responds accordingly
    if (count > 0) and (keyword == 'weather'):
        response_sentence = trigger_speech('weather')
        play(text_to_wav('en-US-Wavenet-F', response_sentence))

    # If the user ask J-Bot for the air quality, she responds accordingly
    elif (count > 0) and (keyword == 'air pollution'): 
        response_sentence = trigger_speech('air pollution')
        play(text_to_wav('en-US-Wavenet-F', response_sentence))

    '''
    The user stands in front J-Bot and asks her how he/she look.
    J-Bot answers according the weather and the air quality
        e.g. If it is cold and the user just wear a shirt, then J-Bot suggests a jacket
    '''
    elif (count > 0) and (keyword == 'how do I look'):
        response_sentence = trigger_speech('camera')
        play(text_to_wav('en-US-Wavenet-F', response_sentence))

    # J-Bot "sleeps" if the use says bye-bye
    elif (count > 0) and (keyword == 'bye-bye robot'):
        play(text_to_wav('en-US-Wavenet-F', "Okay see you later... "))
        count = 0
