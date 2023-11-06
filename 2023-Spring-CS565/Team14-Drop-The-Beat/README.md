[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/cZeLoKcq)

# DropTheBeat: Controlling the Spotify desktop player on an Ubuntu system using Arduino and TinyML

## File explanations:

data:

Different files with collected motion data for gestures in csv format.

model_building.ipynb:

The Python notebook that was used to build the model. One needs to upload the data from the data file to ones own Google Drive and then specify the path to be able to run the notebook out of the box. Further explanations in the notebook.

arduino:

The arduino folder contains 2 different logics that can be uploaded to the Arduino. The data collection file which can be uploaded to collect data as decribed in the final report. The classifier file can be uploaded to use the classifier which has been trained in model_building.ipynb

python:

Script for receiving Bluetooth advertisements from the Arduino and communicating with the Spotify app. !! Runs on Ubuntu. Tested on Ubuntu 22.04 LTS. Can be run executing the command "python3 process_bluetooth_data.py" in the console whith path being the python folder. If running, and arduino/classifier is uploaded to the Arduino, one needs to do one motion every 30 seconds, otherwise the script will stop due to a timeout error. But if one does a motion every 30 seconds and the Spotify app is open it should work without any issues.