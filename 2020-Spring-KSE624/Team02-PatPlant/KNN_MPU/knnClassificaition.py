import pandas as pd 
import datetime
import numpy as np
import matplotlib.pyplot as plt

import time
import datetime
import threading

from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import confusion_matrix
from sklearn.cluster import KMeans
from sklearn.model_selection import train_test_split
from sklearn import tree
import pickle

dir = './'
fileList = ['gyro']

def getSensorData(type):
    stack = []
    for f in fileList:
        data = pd.read_csv(dir + f + '.csv')

        #Timestamp adjustment
        sampleRate = 427
        data['timestamp'] = data.index * sampleRate
        data = data.drop(['timeStamp','type','state'], axis=1)

        #Extract the data that I need (depending on the type)
        if type == 'acc':
            #Ax,Ay,Az
            data = data.drop(['Gx','Gy','Gz'], axis=1)
            data.rename(columns={'Ax':'x', 'Ay':'y', 'Az':'z'}, inplace=True)
        else:
            #Gx,Gy,Gz
            data = data.drop(['Ax','Ay','Az'], axis=1)
            data.rename(columns={'Gx':'x', 'Gy':'y', 'Gz':'z'}, inplace=True)
        
        data['mag'] = np.sqrt(data['x'] ** 2 + data['y'] ** 2 + data['z'] ** 2)

        if len(stack) == 0:
            stack = data
        else:
            stack = pd.concat([data, stack], ignore_index=True)
    return stack


def timedomain_scaled(data):
    WIN_SIZE_IN_MS = 3000
    OVERLAP_RATIO = 0.25
    START_TIME, END_TIME = data['timestamp'].min(), data['timestamp'].max() 

    FEATURES_TIME = []
    WINDOWS = np.arange(START_TIME + WIN_SIZE_IN_MS, END_TIME, WIN_SIZE_IN_MS * (1 - OVERLAP_RATIO))

    for w in WINDOWS:
        win_start, win_end = w - WIN_SIZE_IN_MS, w

        for var in ['x', 'y', 'z', 'mag']:
            # select the rows that belong to the current window, w
            value = data.loc[(win_start <= data['timestamp']) & (data['timestamp'] < win_end), var].values
            #print(value)
            
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


def KMeanCulster(data):
    x = data.drop(columns=['timestamp'])

    model = KMeans(n_clusters=3, algorithm='auto')
    model.fit(x)

    return model


def testModel(model, data):
    x = data.drop(columns=['timestamp'])

    predict = pd.DataFrame(model.predict(x))
    predict.columns=['predict']
    r = pd.concat([x, predict], axis=1)

    plt.scatter(r['Mean-mag'], r['Max-mag'], c=r['predict'],label=r['predict'], alpha=0.5)
    plt.show()


def main():
    #Get Acc. feature time scaled datas (windows)
    FEATURES_TIME_SCALED = timedomain_scaled(getSensorData('gyro'))

    #KNN Clustering to 3 groups
    model = KMeanCulster(FEATURES_TIME_SCALED)

    #Visualization of the result
    testModel(model, FEATURES_TIME_SCALED)

    #save model
    pkl_filename = "shake_classification.pkl"
    with open(pkl_filename, 'wb') as file:
        pickle.dump(model, file)
    

if __name__ == "__main__":
    main()

