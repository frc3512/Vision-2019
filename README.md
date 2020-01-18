## Vision 2019

## ------BUILDING------
To build your project, run "./make.py build" in your terminal. This will download Maven dependencies and Raspbian 9 toolchain.

## ------DEPLOYING-----
To deploy, connect to the robot, otherwise your Raspberry Pi will nots have any power. Open up FRCVision with your Raspberry Pi IP address (the one we have is 10.35.12.3). Open your "Executable" tab and make sure the executable option is set to "C++ executable". Click "Select File" and select your project's executable (in this case, it would be called cvTest) and click "Open".

## Tuning
For now, to capture camera output, go to the web server and screenshot the camera feed in its MJPEG server..

## PiCamera.grip
This is the file that contains our pipeline steps.
Run GRIP and open this file for modification.
