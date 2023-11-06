from flask import Flask
from flask_restful import Resource, Api
from flask_restful import reqparse

import pandas as pd
import datetime
import numpy as np

import time
from time import sleep
import datetime
import threading

import Jetson.GPIO as GPIO
from jetbot import Robot

from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import confusion_matrix
from sklearn.cluster import KMeans
from sklearn.model_selection import train_test_split
from sklearn import tree

import pickle

app = Flask(__name__)
api = Api(app)
robot = Robot()

GPIO.setmode(GPIO.BOARD)
GPIO.setup(33, GPIO.OUT, initial=GPIO.LOW)

index = 0
currStatus = -1
mpuStatus = -1
voiceStatus = -1

# Load from file
pkl_filename = 'shake_classification.pkl'
with open(pkl_filename, 'rb') as file:
    pickle_model = pickle.load(file)

data = pd.DataFrame()

def getRealTimeData(index, gyr):
    #MPU6050 functions with mag (Gyro)
    column_titles = ['Date\n(YY:MM:DD)', 'Time\n(HH:MM:SS)', 'Time\n(µs)','Gx', 'Gy', 'Gz']
    current_time = datetime.datetime.now()
    current_gyro = [float(gyr['gx']), float(gyr['gy']), float(gyr['gz'])]
    row = [current_time.strftime('%Y-%m-%d'), current_time.strftime('%H:%M:%S'), current_time.strftime('%f')]
    row = row + current_gyro

    data = pd.DataFrame([row], columns=column_titles)

    #Timestamp fixation (about 430ms) - sampling rate
    sampleRate = 427
    data['timestamp'] = index * sampleRate
    index += 1

    data.rename(columns={'Gx':'x', 'Gy':'y', 'Gz':'z'}, inplace=True)

    data['mag'] = np.sqrt(data['x'] ** 2 + data['y'] ** 2 + data['z'] ** 2)

    data = data.drop(['Date\n(YY:MM:DD)', 'Time\n(HH:MM:SS)', 'Time\n(µs)'], axis=1)
    return data


def timedomain_scaled(data, idx):
    FEATURES_TIME = []
    w = 3000
    win_start, win_end = data.iloc[0,3], data.iloc[6,3]
    
    for var in ['x', 'y', 'z', 'mag']:
        # select the rows that belong to the current window, w
        value = data.loc[(win_start <= data['timestamp']) & (data['timestamp'] < win_end), var].values 
            
        # extract basic features
        min_v = value.min() # min
        max_v = value.max() # max
        mean_v = value.mean() # mean
        std_v = value.std() # std. dev.

        # append each result (w: current window's end-timestamp, extracted feature) as a new row
        FEATURES_TIME.append((w, '{}-{}'.format('Min', var), min_v))
        FEATURES_TIME.append((w, '{}-{}'.format('Max', var), max_v))
        FEATURES_TIME.append((w, '{}-{}'.format('Mean', var), mean_v))
        FEATURES_TIME.append((w, '{}-{}'.format('Std', var), std_v))

    FEATURES_TIME = pd.DataFrame(FEATURES_TIME, columns=['timestamp', 'feature', 'value'])

    # Reshape data to produce a pivot table based on column values
    FEATURES_TIME = FEATURES_TIME.pivot(index='timestamp', columns='feature', values='value').reset_index()

    return FEATURES_TIME


def MPUClassification(gyr):
    global index, data
    data = data.append(getRealTimeData(index = index, gyr = gyr))
    #sleep(0.1)
    index += 1

    # Shake Detection
    if len(data) > 6:
        data = data.reset_index(drop=True)
        window = timedomain_scaled(data, index)
                
        predict = pickle_model.predict(window.drop(columns=['timestamp']))
        # print(predict[0])  #0: hold, 1: flip, 2: Shake
		# The order might change if you make new model (.pkl).
        
        #Moving window
        data.drop(data.head(2).index, inplace=True)

        return int(predict[0]) + 1

    return -1

#Interaction functions
def shake():
    robot.left(speed=0.3)
    time.sleep(0.3)
    robot.right(speed=0.3)
    time.sleep(0.3)
    robot.stop()

def wave():
    robot.left(speed=0.1)
    time.sleep(0.5)
    robot.right(speed=0.1)
    time.sleep(0.5)
    robot.stop()

def setLED(stat):
    GPIO.output(33, stat)

def voiceClassification(voiceData):
    status = 3
    if voiceData > 800:
        status = 5
    
    return status

#Run REST API Server
class CreateUser(Resource):
    def post(self):
        try:
            #Get json data with parser
            parser = reqparse.RequestParser()
            parser.add_argument('gx', type=str)
            parser.add_argument('gy', type=str)
            parser.add_argument('gz', type=str)

            parser.add_argument('ax', type=str)
            parser.add_argument('ay', type=str)
            parser.add_argument('az', type=str)
            
            parser.add_argument('vc', type=str)

            parser.add_argument('type', type=str)
            args = parser.parse_args()

            #Check the data type
            global mpuStatus, voiceStatus
            if args['type'] == 'MPU':
                mpuStatus = MPUClassification(args)                   #MPU State Code => 1: still, 2: Wave, 3: Shake
            elif args['type'] == 'VOICE':
                voiceStatus = voiceClassification(int(args['vc']))    #Voice State Code => 3: none, 5: bark

	    #Exicute functions if status are changed
            status = mpuStatus * voiceStatus  
            if status != currStatus and status > 0:
		currStatus = status
                if status == 5:         #Bark + Still
                    setLED(GPIO.HIGH)
                elif status == 10:      #Bark + Wave
                    setLED(GPIO.HIGH)
                    wave()
                elif status == 15:      #Bark + Shake
                    setLED(GPIO.HIGH)
                    shake()
                elif status == 3:       #none + still
                    setLED(GPIO.LOW)
                elif status == 6:      #none + Wave
                    setLED(GPIO.LOW)
                    wave()
                elif status == 9:      #none + Shake
                    setLED(GPIO.LOW)
                    shake()

            return {'status':status}
        
        except Exception as e:
            return {'error': str(e)}


api.add_resource(CreateUser, '/user')

if __name__ == '__main__':
    app.run(debug=True, host='192.168.0.59')  # set host ip to your ip add

