# S23 CS565 Mini Project
## D-Au-r: Energy-efficient Smart Automatic Door System with Authority Checking
This mini project aims to identify the user with OV7675 (camera module) on Arduino Nano 33 BLE board, where the camera module ON/OFF is controlled by RSSI fluctuations of BLE.
Each codes can be tested with the instructions below. 
Note that this project has not been tested as an end-to-end manner, and the merged code is not included in this repository. 

## RSSI Thresholding for Camera ON/OFF
Please install **nRF Connect for Mobile** app, which can be easily found on Google Play.
Run _RSSI_thresholding.ino_ on Arduino IDE, then open the Serial Monitor for checking.
Open the app installed above, and find the service named **Battery Level**.
Once you found it, connect to this service, then you can see that Serial Monitor will print the RSSI values with respect to your phone's movement.

## Data Collection from OV7675
Run _arduino_camera.ino_ on Arduino IDE, then open the Serial Monitor for collecting the image data.
Once you send "c" character to the Serial input, the OV7675 will take a image and print the hexadecimal bytes of corresponding image.
Open the _image_check_and_save.ipynb_ for checking the image how it looks and saving as an numpy format. 
Please follow the instructions on this notebook file. 

## Tiny ML for Identification
Please follow the instructions on _tinyML_for_identification.ipynb_ notebook file.


[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/cZeLoKcq)
