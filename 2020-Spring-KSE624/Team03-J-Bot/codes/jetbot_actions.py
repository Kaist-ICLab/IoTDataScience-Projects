#!/usr/bin/env python
# coding: utf-8

'''
Final Project for KSE624 Mobile and Pervasive Computing for Knowledge Services Spring 2020 at KAIST

Last Updated Date: July 01 2020
Authors: 
    Rafikatiwi Nur Pujiarti
    Willmer R. Quinones

-----------------------------

jetbot_actions.py

(1) speech_to_text:
    - Recognize the user's speech and convert it to text
(2) get_date:
    - Get the current date and hour
(3) greeting_context:
    - Select how J-Bot greets the user
(4) air_context:
    - Select what J-Bot will tell the user about the air quality
(5) weather_context:
    - Select what J-Bot will tell the user about the weather condition
(6) recommend_clothes:
    - Check if the user's outfit is suitable for the weather condition, temperature, and air quality
(7) text_to_wav:
    - Convert text to wav file
(8) play:
    - Play the wav file
(9) trigger_speech:
    - Respond to the user's command

'''

## Necessary Packages
from google.cloud import texttospeech
import json
import datetime
from datetime import datetime
from datetime import date
from datetime import time
import playsound as ps
import speech_recognition as sr
from weather_callAPI import get_outside_condition
from clothes_recognition import detectClothes
import traitlets
from IPython.display import display
import ipywidgets.widgets as widgets
from jetbot import Camera, bgr8_to_jpeg
from torch import (cuda, device, load)


# Loading the PyTorch model 
device = device("cuda" if (cuda.is_available()) else "cpu")
clothe_model = load('models/clothe_model.pkl').to(device)

def speech_to_text():

    '''

    Using the speech_recognition library from Python
        source: https://www.codementor.io/@edwardzionsaji/simple-voice-enabled-chat-bot-in-python-kt2qi5oke

    -----------------------------

    Recognize the user's speech and convert it to text

    Returns:
        word: (str) the speech converted to text

    '''

    r = sr.Recognizer()
    with sr.Microphone() as source:
        print("Tell me something:")
        audio = r.listen(source)
        try:
            word = r.recognize_google(audio)
            print("You said:- " + r.recognize_google(audio))
            return word
        except sr.UnknownValueError:
            print("Could not understand audio")
        
def get_date():

    '''

    Get the current date and hour to save the speech audio

    Returns:
        (str) current 'month-day-time'

    '''

    today = date.today()
    current_time = datetime.datetime.now().strftime("%H:%M:%S")
    return f'{today.month}-{today.day}-{current_time}'

def greeting_context():

    '''

    According to the time of the day, select how J-Bot greets the user

    Returns:
        (str) Either 'good morning', 'good day', 'good afternoon', or 'good evening'

    '''

    # Get the current time (24-hours)
    now = datetime.datetime.now()

    # Returns the greeding according to the hour
    if now.hour < 12:
        return 'Good Morning.'
    elif (now.hour == 12) and (now.minute == 0):
        return 'Good Day'
    elif now.hour < 17:
        return 'Good Afternoon.'
    else:
        return 'Good Evening.'

def air_context(status):

    '''

    According to the air quality status, select what J-Bot will tell the user about it

    Args:
        status: (str) Air quality

    Returns:
        (str) What J-Bot tells about the air quality

    '''

    if status == 'Good':
        return 'The air quality today is good.'
    elif status == 'Moderate': 
        return 'There is a little pollution outside today. I advise to wear a mask.'
    elif status == 'Unhealthy for Sensitive Groups':
        return 'The air quality is bad today. Wear a mask. Stay safe.'
    elif status == 'Unhealthy' or (status == 'Hazardous'):
        return 'The air quality is very bad today. It is better to stay inside.'

def weather_context(temp, highest_temp_forecast, highest_temp_time, forecasted_weather,
                   forecasted_time, forecasted_d, current_w, is_all_day, count_equal):

    '''

    According to the weather condition, select what J-Bot will tell the user about it

    Args:
        temp: (int) current temperature
        highest_temp_forecast: (int) highest temperature expected for the day
        highest_temp_time: (str) time when the highest temperature is expected to occur
        forecasted_weather: (str) forecasted weather condition
        forecasted_time: (str) time when the forecasted weather condition is expected to occur
        forecasted_d: (dict) hourly forecasted weather information
        current_w: (str) current weather condition
        is_all_day: (bool) whether the current weather condition stays the same the whole day
        count_equal: (int) counter to check whether the weather remains the same the whole day

    Returns:
        (str) What J-Bot tells about the weather

    '''

    # If the weather condition remains the same during the day, just tells the user about the current condition
    if count_equal == len(forecasted_d):
        if current_w in ['snowing', 'raining', 'cloudy']:
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees and it is going to be {current_w} all day. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                sentence = f"Today is {temp} degrees and it is going to be {current_w} all day."

            if current_w in ['snowing', 'raining']:
                return f"{sentence} Don't forget to bring your umbrella!"
            else:
                return sentence
        elif current_w == 'clear':
            if temp != highest_temp_forecast:
                return f"Today is {temp} degrees and the sky will be {current_w} all day. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                return f"Today is {temp} degrees and the sky will be {current_w} all day."
        else:
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees and there will be {current_w} all day. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                sentence = f"Today is {temp} degrees and there will be {current_w} all day"
            if current_w == 'showers':
                return f"{sentence} Don't forget to bring your umbrella!"
            else:
                return sentence
    
    # If the weather conditionmight change at certain time, warns the user
    else:
        if current_w in ['snowing', 'raining', 'cloudy']:
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees and it is currently {current_w}. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                sentence = f"Today is {temp} degrees and it is currently {current_w}."

            if (current_w != forecasted_weather) and (forecasted_weather in ['snowing', 'raining', 'cloudy']):
                if forecasted_weather in ['raining', 'snowing']:
                    return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!" 
                else:
                    if current_w in ['raining', 'snowing']:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
                    else:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}."
                    
            elif forecasted_weather == 'clear':
                if current_w in ['raining', 'snowing']:
                    return f"{sentence}. The weather might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
                else:
                    return f"{sentence}. The weather might be {forecasted_weather} at {forecasted_time}."
            elif forecasted_weather == 'showers':
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
            
        elif current_w == 'clear':
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees and the sky is clear. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}"
            else:
                sentence = f"Today is {temp} degrees and the sky is clear."

            if forecasted_weather in ['snowing', 'raining', 'cloudy']:
                if forecasted_weather in ['raining', 'snowing']:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!" 
                else:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}." 
            elif forecasted_weather == 'showers':
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
            else:
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}."
            
        elif current_w == 'showers':
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees with {current_w}. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                sentence = f"Today is {temp} degrees with {current_w}."
                
            if forecasted_weather in ['snowing', 'raining', 'cloudy']:
                return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
            elif forecasted_weather == 'clear':
                return f"{sentence}. The weather might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
            else:
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
        
        elif current_w in ['mist', 'smoke', 'haze', 'dust', 'fog']:
            sentence = ""
            if temp != highest_temp_forecast:
                sentence = f"Today is {temp} degrees and there is {current_w}. The highest temperature will be {highest_temp_forecast} at {highest_temp_time}."
            else:
                sentence = f"Today is {temp} degrees and there is {current_w}."
                
            if forecasted_weather in ['snowing', 'raining', 'cloudy']:
                if forecasted_weather in ['raining', 'snowing']:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!" 
                else:
                        return f"{sentence}. It might be {forecasted_weather} at {forecasted_time}."
            elif forecasted_weather == 'clear':
                return f"{sentence}. The weather might be {forecasted_weather} at {forecasted_time}."
            elif forecasted_weather == 'showers':
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}. Don't forget to bring your umbrella!"
            else:
                return f"{sentence}. There might be {forecasted_weather} at {forecasted_time}."


def recommend_clothes(highest_temperature, forecasted_weather, air_quality, top, bot):

    '''

    Check if the user's outfit is suitable for the weather condition, temperature, and air quality.
        - The outfit classes are: 'shirt', 'thin jacket', 'thick clothes', 'long pants', and 'shorts'
        - If it is raining or it will rain, remind the user to bring umbrella
        - If the air quality is not good, encourage the user to wear a mask

    Args:
        highest_temperature: (int) Highest temperature expected during the day
        forecasted_weather: (str) Forecasted weather condition
        air_quality: (str) Classification of the air quality
        top: (str) Upper body outfit of the user
        bot: (str) Lower body outfit of the user

    Returns:
        (str) What J-Bot tells about the user's outfit

    '''

    # Alert the user if it is raining
    if (forecasted_weather == 'raining'):
        raining_alert = "Oh, one more thing, it might rain, so don't forget your umbrella!"
    else:
        raining_alert = ''

    # Alert the user about the air quality
    if (air_quality != 'Good'):
        air_alert = "And do not forget your mask. Take care!"
    else:
        air_alert = ''

    # Check if the user's outfit is suitable given the weather condition
    if (highest_temperature >= 25):
        if (top in ['thick clothes', 'thin jacket']):
            clothe_string = "It's very hot outside, consider taking off that jacket."
        elif (top in 'shirt'):
            clothe_string = "You look great. Have a nice day!"
    elif (highest_temperature < 25) and (highest_temperature >= 18):
        if (top in 'shirt'):
            clothe_string = "It's quite chill today. Wear a jacket!"
        else:
            clothe_string = "You look great. Have a nice day!"
    elif (highest_temperature < 18) and (highest_temperature > 13):
        if (top in 'shirt') and (bot in 'shorts'):
            clothe_string = "It's cold. I think you should wear a proper jacket and long pants."
        elif (top in 'shirt'):
            clothe_string = "It's cold. I think you should wear a proper jacket."
        elif (bot in 'shorts'):
            clothe_string = "It's cold. How about wearing long pants?"
        else:
            clothe_string = "You look great. Have a nice day!" 
    else:
       if (top in 'shirt') and (bot in 'shorts'):
            clothe_string = "It's cold. I think you should wear a proper jacket and long pants."
       elif (top in 'shirt'):
            clothe_string = "It's cold. I think you should wear a proper jacket."
       elif (bot in 'shorts'):
            clothe_string = "It's cold. How about wearing long pants?"
       elif (top in 'thin jacket'):
            clothe_string = "It's cold outside. I think you should wear thicker jacket."
       else:
            clothe_string = "You look great. Have a nice day!" 

    return clothe_string + ' ' + raining_alert + ' ' + air_alert

def text_to_wav(voice_name, text):

    '''

    Using Google Cloud Platform to handle the text-to-speech task
        source 1: https://codelabs.developers.google.com/codelabs/cloud-text-speech-python3/index.html?index=..%2F..index#8
        source 2: https://cloud.google.com/text-to-speech/docs/reference/libraries

    You will need your own Google Cloud Platform authentication file

    -----------------------------

    Convert the text to wav file

    Args:
        voice_name: (str) name for the wave file
        text: (str) The text that is converted to speech

    Returns:
        filename: (str) name of the wave file

    '''

    language_code = '-'.join(voice_name.split('-')[:2])
    output_name = f'output-{get_date()}'
    
    # Instantiates a client with your Google Cloud Platform authentication json file
    client = texttospeech.TextToSpeechClient.from_service_account_json("<YOUR_AUTHENTICATION_FILE.json>")

    # Set the text input to be synthesized
    synthesis_input = texttospeech.SynthesisInput(text=text)

    # Build the voice request, select the language code ("en-US") and the ssml
    # voice gender ("neutral")
    voice = texttospeech.VoiceSelectionParams(
        language_code=language_code, name = voice_name)

    # Select the type of audio file you want returned
    audio_config = texttospeech.AudioConfig(
        audio_encoding=texttospeech.AudioEncoding.LINEAR16
    )

    response = client.synthesize_speech(input=synthesis_input, voice=voice, audio_config=audio_config)

    filename = f'{output_name}.wav'
    with open(filename, 'wb') as out:
        out.write(response.audio_content)
        print(f'Audio content written to "{filename}"')
        
    out.close()
    return filename


def play(filename):

    '''

    Play the wav file (from text-to-speech)

    Args:
        filename: name of the wave file

    '''

    ps.playsound(filename)


def trigger_speech(trigger_type):

    '''

    Respond to the user command, or trigger
        - "greeting": The user says hi to J-Bot and she responds about general weather information
        - "weather": The user asks about the weather, J-Bot responds with the weather information
        - "air pollution": The user asks about the air quality, J-Bot responds with the air quality information
        - "camera": The user asks for the opinion about the outfit, J-Bot responds according to the condition outside
                    whether the outfit is suitable

    Args:
        trigger_type: (str) Command by the user

    Returns:
        (str) What J-Bot tells the user according to what the user asks

    '''
    
    # Get the information about the weather and air quality
    d = get_outside_condition()

    temp = int(d['temperature'])
    highest_temp_forecast = 0
    highest_temp_time = 0
    forecasted_weather = ""
    forecasted_time = ""
    current_w = d['weather']['detail']
    is_all_day = True
    forecasted_data = d['forecast']
    air_quality = d['air_condition']['level']
    
    count_equal = 0
    for f in forecasted_data:
        # Get the highest temperature for the day
        if highest_temp_forecast < f['temperature']:
            highest_temp_forecast = f['temperature']
            highest_temp_time = f['time']
        if current_w == f['weather']['detail']:
            count_equal += 1
        elif (is_all_day == True) and (current_w != f['weather']['detail']):
            forecasted_weather = f['weather']['detail']
            forecasted_time = f['time']
            is_all_day = False
    
    # If the highest forecasted temperature is lower than the current weather, assign current temperature as forecasted highest temperature
    if highest_temp_forecast <= temp:
        highest_temp_forecast = temp

    # Call greeting context, weather context (temperature included), air pollution context    
    if trigger_type == "greeting":        
        return greeting_context() + " " + weather_context(temp, highest_temp_forecast, highest_temp_time, forecasted_weather, forecasted_time, forecasted_data, current_w, is_all_day, count_equal) + " " + air_context(d['air_condition']['level'])

    # Call weather context
    elif trigger_type == "weather":        
        return weather_context(temp, highest_temp_forecast, highest_temp_time, forecasted_weather,forecasted_time, forecasted_data, current_w, is_all_day, count_equal)

    # Call air pollution context
    elif trigger_type == 'air pollution':
        return air_context(air_quality)

    # Check the user's outfit and respond accordingly
    elif trigger_type == 'camera':

        # Initiate the camera
        camera = Camera.instance(width=224, height=224)
        camera.start()
        image = widgets.Image(format='jpeg', width=224, height=224)
        camera_link = traitlets.dlink((camera, 'value'), (image, 'value'), transform=bgr8_to_jpeg)
        image_path = 'temp.jpg'

        # Saving the camera image locally
        with open(image_path, 'wb') as f:
            f.write(image.value)
        camera.stop()

        # Classify the upper and lower outfits of the user
        top, bot = detectClothes(image_path, clothe_model)
        print(top, bot)
        f.close()

        return recommend_clothes(highest_temp_forecast, forecasted_weather, air_quality, top, bot)









