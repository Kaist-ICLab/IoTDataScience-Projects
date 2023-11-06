# smartTune

Smart-Tune aims to automatically control the working environment depending on the user’s  activity. Lighting a lamp, launching and stopping music, changing online availability status are all tasks traditionally done manually. The aim of the research is to conduct such activity detection only based on a single IMU sensor. If effective, such a lightweight model can easily be integrated to a computer, phone or a desk for constant working environment adaptation.


The project ranges from data collection to music output control. It includes the following scripts : 

Data collections: 
Arduino program to collect the sensor data and output to the serial monitor
Python program to save data from the serial monitor

Training:
Colab Notebook Python program to process the data and train the model.

Running application:
Arduino program using the generated model for live classification
Python program to control output depending on 


# how to test

In order to use the pretrained model, open the inference.ino sketch and compile to arduino nano ble.
Later execute music.py script with "python music.py" command


In order to retrain the model "gyro_accl.ino" script can be used for data collection. 
The training script is available in the python notebook train.ipynb


# limitations

While effective during testing the current model does not work well in real life applications where the results depend highly on the setting (table, stability, position, orientation). Multiple approaches can be taken to improve the results, however for lack of time none of these have beeen taken:

It might be effective to train the network with more data, recorded at different times with different arduino orientations as well as different tables. Such addition could highly impact the end result by generalizing the classification to different sensor tilt levels and positionment.  

It could also be effective to work with even longer window frames than the 2 seconds as such a time period can not effectively convey the singularity of the different classes. This of course requires more data collection as well. 

An interesting approach would be to rely on segmentation in order to improve the effectiveness of the model. “Using” and “Stressed” are distinguishable by the instability of their gyroscope data and the higher intensity of the accelerometer data, therefore computing features like variance and mean, perhaps even frequency domain features might be an effective method.  
