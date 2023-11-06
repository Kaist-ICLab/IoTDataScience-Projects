# PetPlant

As the number of one-person households is increasing, some are experiencing psychological issues such as depression, loneliness, and lack of social interaction. They are trying to address these issues through raising pets, as studies have shown that doing so may be helpful in dealing with such problems. However, there are many reasons that people can’t spend their time with their pets all day, such as life patterns, work, school. Therefore there are clear needs that they want to know how their pets are doing while they are out.

However, most of the IoT products for pet care are based on a built-in camera to let users monitor their pets. The built-in cameras have caused various issues related to privacy concerns and lack of awareness of the system presence, which leads to anxiety and discontinued usage of the product. One of the comments with this was that they ‘pull the IoT camera plug off when they arrive home’, due to the sense of someone might watch the video.

Therefore, PetPlant provides a pet's status and context by mapping it into an artificial plant.

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/final%20image.PNG)

Demo Link: https://youtu.be/UauxqAE8PQg

Presentation Slide Link: https://docs.google.com/presentation/d/1-Bml_lIzRBmbYameWIaAGRMfbRswZ7_QlEy3a5eEQT4/edit?usp=sharing

## Pre-requirement
### Hardware Components
#### Sensors
Feather board (ESP8266)

MPU6050

Grove Sound Sensor(https://wiki.seeedstudio.com/Grove-Sound_Sensor/)

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/sensor%20image.PNG)

#### Jetbot Side
Optical fiber

Servo motor

LED (any color is fine!)

3D printed components and pot

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/structure.PNG)

### Software (Library)
Scikit-learn, Pandas, NumPy, mpu6050-raspberrypi, Flask

#### Setup
```
pip install pandas, numpy, mpu6050-raspberrypi -y
pip install nose -y
pip install scikit-learn, flask -y
```

## System Flow

### Data Collection
For the data collection to develop a KNN model, we used the MPU6050 and Grove sound sensor and uploaded it to an application that receives and stores the data. 
Codes for the feather board (ESP8266), which sends the data to application, is in both `./Sensor/gyro_toPythonServer` and `./Sensor/sound_toPythonServer` folder.
Also, you could launch the web application by running the `./Data Collection_Web Server/index.html` with the HTML web server. The application helps you to save the data and export it into a CSV format.

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/application%20image.PNG)

### KNN Clustering with Moving windows
After the data collection (`./KNN_MPU/gyro.csv` in this project), we classified the data into 3 groups (i.e., hold, wave, shake) by using KNN Clustering. We used time-domain scaled for each time term windows of collected data and stacked into data list. However, we didn't scale the time-domain scaled data with Min-Max, due to the single window input in real-time classification. We used `from sklearn.cluster import KMeans` to build the KNN model and run `model.fit()` with the scaled data list. As shown in the figure, it classifies 3 groups well and for the real-time data too.

From other MPU library that supports recoding [1], you could build your model by using `python3 ./KNN_MPU/knnClassificaition.py` and `python3 ./KNN_MPU/realTime (Direct Connection).py` to test the real-time data on the model in the local environment.

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/clustering%20result.PNG)

### Real-Time Classification
The overall algorithm for real-time classification uses queue. As the figure below, the `getRealTimeData` function in `Server/server.py` makes the queue to get 1 real-time input per loop and checks the status when the queue contains the proper length (in this code: 6) of data by using `model.predict()`. The model in this code is generated from the `./KNN_MPU/knnClassificaition.py`. After classification, it reacts depending on the result and removes the oldest 2 data from the queue to get new real-time data.

![](https://raw.githubusercontent.com/jinwook31/PetPlant/master/Queue.JPG)

For the real-time classification, we used the Flask server to implement REST API. As we send the data from the feather board to application in data collection, we changed the IP address to the Jetbot and send a JSON format data to the Flask server.

You could run the server by `python3 ./Real Time Class w Server/server.py` to initiate the server that receives data from the sensor and make it as a time-domain scaled window. It classifies the status with the KNN model generated previously(shake_classification.pkl) and provides interaction with manipulating servo motor and LED.

### Interaction
For the interaction, there is a total of 6 combinations (2 led x 3 servo movement). LED represents the voice and the servo movement represents the pet tail movement.

## Reference 
[1] https://github.com/vijaykumarwalikar/mpu6050-data-recording-visualization-tool
