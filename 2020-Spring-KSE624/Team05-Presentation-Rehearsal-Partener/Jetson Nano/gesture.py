###################################################
### THIS PYTHON FILE IS FOR GETTING GESTURES ######
### DATA FROM CLOUD. ##############################
###################################################

import requests
import json 

# explicitly declare the content type of the caller as application/json
headers = { 'Content-Type': 'application/json' }

# server url
base_url = 'http://jet-presentation.koreacentral.cloudapp.azure.com:5000'

# function for getting gestures
def get_gesture(image_path, is_gpu=False):
    """Get gesture classification result.

    Args:
        image_path: path to get image file.
        is_gpu: boolean to check if gpu is used.
        response: gesture.

    Returns:
        response in JSON format.
        {'gesture' : 'walking'}, for example.
    """
    response = requests.post(base_url + '/get-gesture', headers=headers, data=json.dumps({"image_path": image_path, "is_gpu": is_gpu}))

    return response.json()
