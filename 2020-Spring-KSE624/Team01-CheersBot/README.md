# KSE624-Team1
# Cheersbot
Let's Cheers with Cheersbot!

### Watch the Demo video & Slide
[![Video Label](http://img.youtube.com/vi/7Yft4uTX8Ms/0.jpg)](https://youtu.be/7Yft4uTX8Ms=0s)

[Link to Slide](https://docs.google.com/presentation/d/1DhWW_3buYzDjDduUoTDXJvPRc6uU8FgfSCC8IZCqkdQ/edit?usp=sharing)

## introduction
After the Covid 19 Pandemic, people cannot drink outside. People are looking for alternatives such as 'Drinking with friends through video-call'. This new type of drinking culture is beginning to trend.
But-Dr. Priscilla Martinez, from Alcohol Research Group  says, "It is important to know your drinking status." The doctor said that to avoid side effects such as alcoholism, it is necessary to observe the changes in drinking time, reason, and amount of alcohol. 
Much have changed a lot from our daily lives, and the interaction between people is limited. It is getting too long to say that you have to be patient during this period.
Our team thought that enjoying time wisely with fun interaction is what people need.

We focus on the problem that drinking through video call can fills visual and auditory elements, but not physical interaction. Also, since you are drinking alone, it is important to know your drinking status.

## Pre-requirement
### Files needed for audio output & Voice recognition
#### How we get '.wav' file 
1. Visit [NAVER CLOVA](https://clova.ai/voice)
2. You can record sentence with AI voice. record the sentence for '1zzan.mp3', '2zzan.mp3'and so on.
3. If you get .mp3 file, visit the [audio converter website](https://online-audio-converter.com/ko/) to convert .mp3 file to .wav file.
4. Open your file and click the wav button and '변환(convert)'button in order.

#### How we get '.pmdl' file
snowboy provides a personal model. The process is very simple, like this:

1. visit the [Snowboy website](https://snowboy.kitt.ai/)
2. Login in with social media
3. Click the Create Hotword and set your Hotword like "how much glasses I drink?" and record your voice
4. Test the model and click save and download button. Then you can get pmdl (personal model) files

### Hardware Components
![cheersbot_component](https://user-images.githubusercontent.com/44702454/86465579-e38ce200-bd6c-11ea-936a-edb10434db80.png)
+ Jetbot
+ Speaker (for auditory interaction)
+ Glass (for physical interaction)
+ IMU Sensor (for collecting accelerometer data)
+ Medium-density fibreboard(MDF) (for Laser-cutted customized appearance)

### Software setup
### How to download trianed model
Instead of building the machine learning model, we used pre-trained SSD MobileNet V2 model to recognize hand with glass. You need to download this model(ssd_mobilenet_v2_coco.engine). 

This model is trained using COCO-dataset: It is the dataset used for large-scaled object detection. It includes 90 kinds of different objects. 
Among them, we want to recognize two objects:

+ index 0- Person
+ index 47- Cup
+ If a hand with glass is in front of camera, then this model can detect it as index 0 or 47.

You can download the pre-trained engine in [here](https://drive.google.com/file/d/1eZsGracgHo5zSakGMHcW2ET19UtLfDKf/view?usp=sharing)

The model is made from the ObjectDetector API(from TensorFlow).
Once the model is built, NVIDIA TensorRT can optimize it: real-time execution is capable on Jetson Nano. 

**TensorRT is kind of SDK & Toolkit.**  It provides an inference optimizer and runtime engine, so without expertise, you can conduct high-performance inferencing. Using this model, we don't have to train and optimize the model.

#### Installation
1. Please install smbus and FaBoAxis_MPU9250
In your terminal, make sure you have pip3 installled.

```python
sudo apt-get install python3-pip
```

Now clone the github site and install smbus2.

```python
sudo git clone https://github.com/FaBoPlatform/FaBo9AXIS-MPU9250-Python.git
pip3 install smbus2 <code>
```

Install the FaBo9Axis_MPU9250

```
pip3 install FaBo9Axis_MPU9250
```

Refer to the following [link](https://brisbaneroboticsclub.id.au/install-keystudio-i2c-shield-v1/)

2. Install the snowboy and make your own pmdl model
You can do it in following [Snowboy website](https://snowboy.kitt.ai/) 
(You have to register and make your own id)

We made two models: howmuch.pmdl(using 'How much cheers?') and zzanhalgga(using 'Shall we cheers?')
Download the pmdl model and store them in the path which contains your python notebook.

#### Download
1. Download the wav files and store them at the path which contains your notebook.
You can download them in [here](https://drive.google.com/drive/folders/1hhn2-ccdgHKmKi6KiMCy6lB9r9NzFBiP?usp=sharing)

2. Download the 'ssd_mobilenet_v2_coco.engine' dataset(You can easily find it in Google) and store it at the path which contains your notebook.

## How to operate
1. Except last two cells, all of the cells above import the required modules and define the function.
2. If you run a cell containing a while loop, 
	1. If your hand and glass is in front of the Cheersbot, it detects your hand with glass and moves forward.
	2. If you do 'Cheers' interaction with it, it moves backward
	3. After that, say specific sentence(such as 'Shall we cheers?' or 'How much cheers?') then it will give the auditory feedbacks.
	4. Iterate it until the while loop is terminated.
	5. If Cheersbot doesn't stop, excute last cell.
## Overall Flowchart
![image](https://user-images.githubusercontent.com/44702454/86480632-55732480-bd89-11ea-98f5-0b8ee45d7015.png)
### Four Main feature
1. 'Cheers' Interaction
2. Voice Recognition & Feedback
3. Let me know how much I drink
4. Reject cheers if the user drinkd too much


<img src = "https://user-images.githubusercontent.com/44702454/86585474-7cb43680-bfc1-11ea-9a18-9b6d628acc79.png" width="500px"><img src = "https://user-images.githubusercontent.com/44702454/86477490-7fc1e380-bd83-11ea-9535-67d6fa3e4c94.png" width="500px">
<img src = "https://user-images.githubusercontent.com/44702454/86477516-8a7c7880-bd83-11ea-8984-2866039661e9.png" width="500px"><img src = "https://user-images.githubusercontent.com/44702454/86477533-9405e080-bd83-11ea-8440-4bd8b2112f6d.png" width="500px">
<img src = "https://user-images.githubusercontent.com/44702454/86477550-9f590c00-bd83-11ea-8136-dc7f1abef642.png" width="500px"><img src = "https://user-images.githubusercontent.com/44702454/86585550-98b7d800-bfc1-11ea-9393-29447a64a1ed.png" width="500px">

## Laser-cut customized appearance
I use my school's free laser cutting area('IDEA FACTORY' @KAIST) so, This tutorial is dependent on a certain place's condition. In my case,'.DXF file' is needed if you trying to use a laser cutting machine @ IDEA FACTORY.

There were a lot of ways to make a .DXF file. If you are not good at manipulating tools such as AutoCAD, Fusion360 from Autodesk.
I recommend a very useful [website.](https://en.makercase.com/#/) This website provides a basic model as a **.DXF file.** still, it needs a little modification, but You can make **.DXF file** easier.
+ Choose the Box type (Basic Box, Polygon Box, Kerf Bent Box)
+ Set the Units, Width, Height, Depth as you measured before or it could be the sides, size, Inside Diameters, etc.(it's different from the box type)
+ Choose other options below.
+ Click the Download Box Plans and then a new pop up window will be opened
+ Check whether the drawing is what expected.
+ Everything Okay, then click the Download DXF
+ You will get the .DXF file within a few minutes!
+ If you want to modify, open **.DXF file** at AutoCAD or Fusion360 and then modify it as you want.

<img src = "https://user-images.githubusercontent.com/44702454/86588581-eedb4a00-bfc6-11ea-93c2-fd6948b004e2.jpg" width="500px"><img src = "https://user-images.githubusercontent.com/44702454/86588437-a3c13700-bfc6-11ea-884b-f7834560fd69.jpg" width="500px">

I especially consider that Jetbot should be attached with lots of lines and it carries glass. so that I made two-hole for lines and glass.      
(I attached `Cheersbot(ProductDrawing).dxf` file. You can check exact values for two holes)      
Once, you make **.DXF file** bring it to the laser cutting machine. and Then, run **the laser cutting machine.**     
Then, you will get a separate MDF part. Glue the parts with ***super glue.***

