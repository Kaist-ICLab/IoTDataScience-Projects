###################################################
### THIS PYTHON FILE IS FOR GETTING EYE CONTACT ###
### DATA FROM CLOUD. ##############################
###################################################

import requests
import json

# explicitly declare the content type of the caller as application/json
headers = {'Content-Type': 'application/json'}

# server url
base_url = 'http://jet-presentation.koreacentral.cloudapp.azure.com:5000'

# function for getting eye contact


def get_eye_contact(image_path, is_gpu=False):
    """Get eye contact classification result.

    Args:
        image_path: path to get image file.
        is_gpu: boolean to check if gpu is used.
        response: eye contact score.

    Returns:
        response in JSON format. 
        {'score' : '0.83'}, for example.
    """
    response = requests.post(base_url + '/get-eye-contact', headers=headers,
                             data=json.dumps({"image_path": image_path, "is_gpu": is_gpu}))

    return response.json()
