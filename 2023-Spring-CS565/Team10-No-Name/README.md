# CS565_Project

This repository contains resources essential for enhanced object recognition. The main components are organized into separate directories, as described below:

project: This directory holds the Arduino scripts required for raw data collection. These scripts facilitate the gathering of acceleration and angular velocity data from specific objects.

object: Here, you will find raw data sets for each of the nine distinct objects utilized in the project. This data was harvested using the scripts found within the arduino_code directory.

project.ipynb: This Jupyter Notebook contains the main pipeline of the project, starting from model creation to validation and conversion for Arduino compatibility. It comprises the steps of CNN model building, training, validation, and subsequent conversion to .cc files that can be easily utilized in the Arduino environment.

models: All the converted TensorFlow Lite models (tflite files) and their corresponding .cc files are stored in this directory. These files are the result of the conversion process executed in the project.ipynb file.

arduino_deploy: This directory contains the deployable Arduino code. It's designed for direct utilization within the Arduino programming environment, leveraging the .cc files produced from the conversion process.

Please feel free to explore each folder and file to gain a more comprehensive understanding of the project's structure and operations.