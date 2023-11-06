#######################################################################
### THIS PYTHON FILE IS THE MAIN APPLICATION FILE OF JETSON NANO ######
#######################################################################

import traitlets
import time
import os
import firebase_admin
import urllib
import requests as req
import numpy as np
import Jetson.GPIO as GPIO
import ipywidgets.widgets as widgets

from face import get_facial_expression
from eye import get_eye_contact
from gesture import get_gesture
from uuid import uuid1
from firebase_admin import credentials
from google.cloud import storage
from jetbot import Camera, bgr8_to_jpeg
from matplotlib import pyplot as plt 
from matplotlib import image as mpimg 

# start GPIO board for LEDs.
GPIO.setmode(GPIO.BOARD)
# identify the GPIO port(s) to each LED color.
channels = {
    'white': [22, 24],
    'red': 26,
    'yellow': 29,
    'green': [16, 18],
    'blue': [36, 38]
}
# set up the GPIO port(s).
for c in channels.values():
    GPIO.setup(c, GPIO.OUT)

cred = credentials.Certificate("./jet-presentation-firebase-adminsdk.json")
firebase_admin.initialize_app(cred, {
    'storageBucket': 'jet-presentation.appspot.com'
})

os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "./jet-presentation-firebase-adminsdk.json"

get_status_path = 'https://us-central1-jet-presentation.cloudfunctions.net/services/getStatus'
send_result_path = 'https://us-central1-jet-presentation.cloudfunctions.net/services/updateReportData' # firebase

# default username
username = 'jetbot'

def get_current_status():
    """Get current status.

    Args:
        result: user's current status in JSON format
        current_status: user's current status.

    Returns:
        current status.
    """
    result = req.post(get_status_path, { "username": username })
    current_status = result.json()['data']['status']
    return current_status


def save_snapshot(image):
    """save snapshot from Jetson camera.

    Args:
        image: snapshot.
        image_path: path to save snapshot.
    """
    image_path = 'user_image.jpg'
    with open(image_path, 'wb') as f:
        f.write(image.value)

        
def upload_report_data(data):
    """upload report data.

    Args:
        data: report data
    """
    data['username'] = username
    result = req.post(send_result_path, data)
#     print(result.json()['result'])


def upload_image(image_path):
    """upload image.

    Args:
        image_path: path to image file.
        storage_client:
        bucket:
        blob:
        image_url: public image URL from blob
    
    Returns:
        image_url: URL to image
    """
    storage_client = storage.Client()
    bucket = storage_client.bucket('jet-presentation.appspot.com')

    blob = bucket.blob(username + '/' + str(uuid1()) + '.jpg')
    blob.upload_from_filename(image_path)
    image_url = blob.public_url
    return image_url

good_exp = ['happiness', 'neutral', 'surprise'] # good facial expression set
bad_exp = ['anger', 'contempt', 'disgust', 'fear', 'sadness'] # bad facial expression set
bad_gestures = ['point', 'clasp_hands', 'hold', 'cross_arms', 'hide_two_arms', 'hide_one_arm', 'stand_improperly', 'rotate_head', 'touch_body'] # bad gestures set

def check_bad_expression(emotions):
    """check bad expression.

    Args:
        emotions: user's emotion
        bad_scores: counting variable for bad facial expression.

    Returns:
        True (boolean): if user express bad facial expression more than half of a presentation.
    """
    bad_scores = 0
    for bad in bad_exp:
        bad_scores += emotions[bad]
    return bad_scores > 0.5 

# open jetson camera         
camera = Camera.instance(width=512, height=512, fps=5)
image = widgets.Image(format='jpeg', width=224, height=224)
camera_link = traitlets.dlink((camera, 'value'), (image, 'value'), transform=bgr8_to_jpeg)


# constant loop receive current status every 3 seconds
while True:
    current_status = get_current_status() # get user's current status.
    if current_status == 1:
        # if user's current status is in stand by mode (1),
        # WHTIE LED is on. 
        GPIO.output(channels['white'], GPIO.HIGH)
        print('Connected')
    elif current_status == 2:
        # if user's current status is in recording mode (2),
        # user presentation is snapped by Jetbot.
        # WHTIE LED is off, RED LED is on.
        save_snapshot(image)
        GPIO.output(channels['red'], GPIO.HIGH)
        GPIO.output(channels['white'], GPIO.LOW)
        print('Recording')
      

        image_path = upload_image('user_image.jpg') # image path from snapshot of JetBot

        expression = get_facial_expression(image_path) # getting facial expression from the snapshot.

        eye_contact = get_eye_contact(image_path, is_gpu=False) # getting eye contact from the snapshot.

        gesture = get_gesture(image_path, is_gpu=False) # getting gesture from the snapshot.
        
        emotions = None # defualt emotion

        # condition for LED feedback.
        if len(expression):
            emotions = expression[0]['faceAttributes']['emotion']
            # if user's facial expression is in the bad set,
            # BLUE LED is on.
            # Otherwise, BLUE LED is off.
            if check_bad_expression(emotions):
                GPIO.output(channels['blue'], GPIO.HIGH)
            else:
                GPIO.output(channels['blue'], GPIO.LOW)
        else:
            # if model cannot find face in the snapshot,
            # BLUE LED is on.
            GPIO.output(channels['blue'], GPIO.HIGH)

        if eye_contact['score'] < 0.7:
            # if user did not have an eye contact with Jetson camera,
            # YELLOW LED is on.
            # Otherwise, YELLOW LED is off.
            GPIO.output(channels['yellow'], GPIO.HIGH)
        else:
            GPIO.output(channels['yellow'], GPIO.LOW)
        
        if gesture['gesture'] in bad_gestures:
            # if user did the improper gestures,
            # GREEN LED is on.
            # Otherwise, GREEN LED is off.
            GPIO.output(channels['green'], GPIO.HIGH)
        else:
            GPIO.output(channels['green'], GPIO.LOW)
        
        # get emotion data (Top1)
        if emotions:
            emotion = max(emotions, key=emotions.get)
        else:
            emotion = "None"
        
        # make report data (dictionary).
        report_data = {
            "timestamp": int(time.time()),
            "image_url": image_path,
            "eye_contact": eye_contact['score'],
            "emotion": emotion,
            "gesture": gesture['gesture']
        }
        
        print(report_data)
        upload_report_data(report_data)
    elif current_status == 3:
        # if user end the presentation (3: processing report mode),
        # ONLY WHTIE LED is on.
        GPIO.output(channels['white'], GPIO.HIGH)
        all_leds = [channels['red']] + [channels['yellow']] + channels['green'] + channels['blue']
        GPIO.output(all_leds, GPIO.LOW)
        print('Report Processing')
    else:
        # if user disconnect to the Jetson,
        # all LEDs is off.
        all_leds = channels['white'] + [channels['red']] + [channels['yellow']] + channels['green'] + channels['blue']
        GPIO.output(all_leds, GPIO.LOW)
        print('Not Connected')
    
    if current_status != 2:
        # if user's status is not in recording mode (2),
        # the system will wait 3 seconds before getting the new status.
        time.sleep(3)
    
GPIO.cleanup() # clear GPIO board.