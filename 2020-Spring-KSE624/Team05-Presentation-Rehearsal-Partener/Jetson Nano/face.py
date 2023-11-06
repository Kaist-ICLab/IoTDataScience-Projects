###################################################
### THIS PYTHON FILE IS FOR GETTING FACIAL ########
### EXPRESSION DATA FROM CLOUD. ###################
###################################################

import os
import uuid
import time
import requests

# server url
face_api_url = 'https://jet-face.cognitiveservices.azure.com/face/v1.0/detect'

# subscription key for azure authentication
headers = {'Ocp-Apim-Subscription-Key': '<YOUR SUBSCIPRTION KEY>'}

# option for returned values from the API.
params = {
    'returnFaceId': 'false',
    'returnFaceLandmarks': 'false',
    'returnFaceAttributes': 'smile,glasses,emotion,makeup,accessories',
}

# function for getting facial expression
def get_facial_expression(image_path):
    """Get predictied facial expression.

    Args:
        image_path: path to get image file.
        response: facial expression.

    Returns:
        response in JSON format.
        {'emotion' : 'neutral'}, for example.
    """

    response = requests.post(face_api_url , headers=headers, json={'url': image_path}, params=params)

    return response.json()
