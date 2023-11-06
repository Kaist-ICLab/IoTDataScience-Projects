######################################################
### THIS PYTHON FILE IS THE MAIN FLASK APPLICATION ###
### THAT USED FOR EYE CONTACT AND GESTURE PREDICTION #
######################################################

# Import related modules and libraries
import dlib
import cv2
import argparse
import os
import random
import requests
import uuid
import json
import torch
import urllib
import torch.nn as nn
import torch.nn.functional as F
import torchvision
import pandas as pd
import numpy as np

from flask import Flask, render_template, url_for, jsonify, request
from tensorflow.keras.models import load_model
from tensorflow.keras.preprocessing import image
from tensorflow.keras.applications.mobilenet_v2 import preprocess_input
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from colour import Color
from matplotlib import pyplot as plt
from matplotlib import image as mpimg
from torchvision import datasets, transforms
from model import model_static

# transform image into tensor with the given size
test_transforms = transforms.Compose([transforms.Resize(224),
                                      transforms.CenterCrop(224),
                                      transforms.ToTensor(),
                                      transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[
                                                           0.229, 0.224, 0.225])
                                     ])

# load trained gesture model
gesture_model = load_model('./gesture_model.h5')


def download_image(URL):
    """Download an image from given url.

    Args:
        URL: image url.

    Returns:
        preprocessed image data.
    """
    with urllib.request.urlopen(URL) as url:
        with open('user_image.jpg', 'wb') as f:
            f.write(url.read())
    img = image.load_img('user_image.jpg', target_size=(224, 224))
    return preprocess_input(image.img_to_array(img))


app = Flask(__name__)
app.config['JSON_AS_ASCII'] = False


@app.route('/')
def index():
    # index path return simple string
    return 'Hello, World! -- Welcome to Jet Presentation!'


@app.route('/get-gesture', methods=['POST'])
def get_gesture():
    """Classify the given input image url and return its label.

    Returns:
        label of the given image.
    """

    # class name for a given index.
    classes = ['call_me', 'clasp_hands', 'cross_arms', 'hide_one_arm', 'hide_two_arms', 'hold',
    'list', 'move', 'open_one_arm', 'open_two_arms', 'point', 'roll',
    'rotate_head', 'show_level', 'show_small_thing', 'stand_improperly', 'stand_properly', 'touch_body']

    # get request body data in json format
    data = request.json
    image_path = data['image_path']
    is_gpu = data['is_gpu']

    # preprocess input image as a input for the gesture model
    x = np.array([download_image(image_path)])

    # get label index
    gesture_idx = np.argmax(gesture_model.predict(x), axis=-1)[0]
    return jsonify({'gesture': classes[gesture_idx]})


@app.route('/get-eye-contact', methods=['POST'])
def get_eye_contact():
    """Detect the given input image url and return its classification result wheter eye contact or not.

    Returns:
        eye contact detect result of the given image.
    """

    data = request.json
    image_path = data['image_path']
    is_gpu = data['is_gpu']
    
    # load model weights
    model_weight = "data/model_weights.pkl"
    model = model_static(model_weight, is_gpu=is_gpu)
    model_dict = model.state_dict()

    if is_gpu:
        snapshot = torch.load(model_weight)
    else:
        snapshot = torch.load(model_weight, map_location=torch.device('cpu'))
    model_dict.update(snapshot)
    model.load_state_dict(model_dict)

    if is_gpu:
        model.cuda()
    model.train(False)

    # load cnn model 
    modelFile = "data/opencv_face_detector_uint8.pb"
    configFile = "data/opencv_face_detector.pbtxt"
    net = cv2.dnn.readNetFromTensorflow(modelFile, configFile)

    # read the image url to memory
    frame = mpimg.imread(urllib.request.urlopen(image_path), format='jpeg')
    frameWidth = frame.shape[1]
    frameHeight = frame.shape[0]

    blob = cv2.dnn.blobFromImage(frame, 1.0, (300, 300), [104, 117, 123], False, False)
    net.setInput(blob)
    detections = net.forward()

    bbox = []
    for i in range(detections.shape[2]):
        confidence = detections[0, 0, i, 2]
        if confidence > 0.5: #conf_threshold
            x1 = int(detections[0, 0, i, 3] * frameWidth)
            y1 = int(detections[0, 0, i, 4] * frameHeight)
            x2 = int(detections[0, 0, i, 5] * frameWidth)
            y2 = int(detections[0, 0, i, 6] * frameHeight)
            bbox.append([x1,y1,x2,y2])

    frame = Image.fromarray(frame)
    score = 0
    for b in bbox:
        face = frame.crop((b))
        img = test_transforms(face) #img -> tensor

        img.unsqueeze_(0)

        if is_gpu:
            output = model(img.cuda())
        else:
            output = model(img)

        # get classification output score
        score = F.sigmoid(output).item()
        score = round(score, 2)

    return jsonify({"score": score})
