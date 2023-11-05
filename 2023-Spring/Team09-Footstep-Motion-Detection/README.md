# Footstep Motion Detection for Tetris Anywhere, Anytime

This project enables motion capture for exergames using wearable socks equipped with sensors. By capturing the movement of users' feet, the system allows for interactive gameplay without spatial constraints. The repository contains the necessary code and data to implement the motion capture system.

## Repository Structure

The repository is structured as follows:

```
├── app/
│   ├── app.ino
│   ├── arduino_main.cpp
│   ├── arduino_output_handler.cpp
│   ├── main_functions.h
│   ├── model.cpp
│   ├── model.h
│   └── output_handler.h
├── collection/
│   └── collection.ino
├── data/
│   └── 0/
│       ├── l0.csv
│       ├── l1.csv
│       ├── l2.csv
│       ├── r0.csv
│       ├── r1.csv
│       └── r2.csv
├── models/
│   ├── model.cc
│   ├── model.tflite
│   └── model_no_quant.tflite
├── README.md
└── model.ipynb
```
- The `app/` directory contains the Arduino code and functions required for the motion capture system.
- The `collection/` directory includes an Arduino script for collecting motion data from the wearable socks.
- The `data/` directory contains example motion data collected from the socks, divided into separate folders for each user (e.g., `0/`).
- The `models/` directory contains the pre-trained motion capture models in TensorFlow Lite format.
- The `README.md` file provides an overview of the project and instructions for using the repository.
- The `model.ipynb` notebook contains code for training and evaluating the motion capture model.
## Getting Started

To use this motion capture system, follow these steps:

1. Set up the Arduino environment and connect the wearable socks with the sensors.
2. Upload the code from the `app/` directory to the Arduino board.
3. Use the `collection/` script to collect motion data from the wearable socks.
4. Store the collected data in the `data/` directory, organizing it into folders based on user ID.
5. Optionally, use the `model.ipynb` notebook to train and evaluate a custom motion capture model.
6. Store the trained model in the `models/` directory.
7. Modify the Arduino code in `app/` to use the desired model and implement any additional functionality.
8. Upload the modified Arduino code to the board to enable real-time motion capture and interaction.

## Dependencies

### Arduino Dependencies

| Library Name | Installation |
|--------------|--------------|
| Arduino_LSM9DS1 | [Link to Library](https://github.com/arduino-libraries/Arduino_LSM9DS1) |
| TensorFlowLite | [Link to Library](https://github.com/tensorflow/tflite-micro) |
| Keyboard | [Link to Library](https://www.arduino.cc/reference/en/libraries/keyboard/) |

#### Arduino_LSM9DS1

To install the Arduino_LSM9DS1 library, follow these steps:

1. Open the Arduino Library Manager (Sketch > Include Library > Manage Libraries).
2. Search for "Arduino_LSM9DS1" and click the "Install" button.

#### TensorFlowLite

To use TensorFlow Lite on Arduino, follow the instructions provided in the TensorFlow Lite Arduino documentation: [Link to Documentation](https://www.tensorflow.org/lite/microcontrollers/arduino).

### Python Dependencies
```
pip install tensorflow pandas numpy
pip install plotly  # if you want to use Plotly for visualizations
```

Please note that the code assumes you have access to the specific dataset files referenced by their Google Drive IDs. You may need to replace these IDs with your own data file IDs or modify the code to load your data from a different source.

## Dataset

The dataset used for training the model consists of sensor data collected from various activities. The dataset includes data files for different activities such as standing, stepping, and centering on both the right and left sides. The data is loaded, preprocessed, and split into training and testing sets.

## Training

The model is designed as a CNN using TensorFlow's Keras API. It consists of convolutional layers, batch normalization layers, max pooling layers, and dense layers. The model is compiled with appropriate loss and optimization functions. It is then trained on the training set for a specified number of epochs.

## Model Conversion

After training, the model is converted to TensorFlow Lite format for deployment on resource-constrained devices. Two versions of the model are generated: one without quantization and one with quantization. The quantized model is optimized for reduced model size and improved performance on low-power devices.

## Testing

To evaluate the performance of the trained models, they are tested on the testing set. The accuracy of the predictions is calculated and compared between the models.

## Usage

To use the trained and converted TensorFlow Lite models, you can follow the provided code snippets and examples in the notebook. The TensorFlow Lite model can be deployed on resource-constrained devices using appropriate frameworks or libraries for inference.

## References
For more information on related research and technologies, please refer to the following references:

- Yan, Z., Zhou, J., Wu, Y., Liu, G., Luo, D., Zhou, Z., ... & Wang, G. (2022). Shoes++ A Smart Detachable Sole for Social Foot-to-foot Interaction. Proceedings of the ACM on Interactive, Mobile, Wearable and Ubiquitous Technologies, 6(2), 1-29.
- Nam, S. H., & Kim, J. Y. (2018). Dance exergame system for health using wearable devices. IEEE Access, 6, 48224-48230.
- Kim, M., & Lee, D. (2017). Development of an IMU-based foot-ground contact detection (FGCD) algorithm. Ergonomics, 60(3), 384-403.

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/cZeLoKcq)
