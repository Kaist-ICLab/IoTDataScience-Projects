import serial
import numpy as np
import tensorflow as tf
from tensorflow import keras

# 시리얼 통신 설정
ser = serial.Serial('COM5', baudrate=9600)  # 시리얼 포트와 속도를 적절히 설정해야 합니다.

# 입력 데이터 크기
input_size = (119, 8)

# 종료 조건 설정
max_iterations = 5000  # 최대 반복 횟수
iteration = 0  # 현재 반복 횟수

model = keras.models.load_model('model.h5')
data_list = []

classes = ['CERAMIC', 'FABRIC', 'GLASS', 'ALUMINIUM', 'PAPER', 'PLASTIC_BAG', 'PLASTIC', 'STAINLESS', 'WOOD']

while iteration < max_iterations:
    # Reading the data from the serial
    data_string = ser.readline().decode().strip()
    
    # Splitting and removing any empty strings
    data_string = list(filter(None, data_string.split(',')))
    
    # Converting the data to float
    try:
        data = list(map(float, data_string))
    except ValueError as ve:
        print(f"Error converting data to float: {ve}")
        continue

    # drop first value (timestamp)
    data = data[1:]
    
    # Computing the acceleration and gyroscope intensities
    acc_intensity = np.sqrt(data[0]**2 + data[1]**2 + data[2]**2)
    gyr_intensity = np.sqrt(data[3]**2 + data[4]**2 + data[5]**2)
    
    # Appending intensities to the data
    data.append(acc_intensity)
    data.append(gyr_intensity)

    data_list.append(data)
    
    # Creating a numpy array from the list

    # preprocessed_data = np.array(data)

    # print(preprocessed_data)

    if len(data_list) == input_size[0]:
        # Convert the list to a numpy array and reshape it
        input_data = np.array(data_list).reshape(input_size)
        # print(input_data)

        # Feeding the input data into the model and getting the predicted class
        predictions = model.predict(np.expand_dims(input_data, axis=0))
        print(predictions)
        predicted_class = np.argmax(predictions)

        # # Printing the predicted class
        print(f"Predicted class: {classes[predicted_class]}")

        # Clear the data list for the next set of samples
        data_list = []

    iteration += 1

# Closing the serial communication
ser.close()
