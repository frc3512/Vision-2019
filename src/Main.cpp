#include <vector>
#include <cscore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cameraserver/CameraServer.h>

#include "GripPipeline.h"
#include "opencv4/opencv2/opencv.hpp"
#include "UdpSocket.hpp"

uint32_t GetIP(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3){
	return (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
}

int main() {
    GripPipeline pipe;
    cs::UsbCamera camera = CameraServer::GetInstance().StartAutomaticCapture(); // creates camera named "USB Camera 0"
    cs::CvSink cvsink = CameraServer::GetInstance().GetVideo();

    uint16_t socketPort = 5001;
    uint32_t ip = GetIP(10, 35, 12, 2);
    UdpSocket socket;
    cv::Mat input;

    // changed camera to dark
    camera.SetVideoMode(PixelFormat.kYUYV, 320, 240, 30)
    camera.SetResolution(160, 120);
    camera.SetFPS(15);
    socket.bind(socketPort);

    // Camera internals
    cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
    double focalLengthx = input.cols;
    double focalLengthy = input.rows;
    cv::Point2d center = cv::Point2d(input.cols/2, input.rows/2);
    cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << focalLengthx, 0, center.x, 0, focalLengthy, center.y, 0, 0, 1);
    cv::Mat distortionMatrix = cv::Mat::zeros(4,1,cv::DataType<double>::type); // no distortion assumed

    // Main loop
    while (){
        cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
        pipe.Process(input);
        cv::Mat output = pipe.GetHsvThresholdOutput();

        std::vector<cv::KeyPoint> points = pipe.findBlobsOutput;
        std::vector<cv::Point2f> imagePoints;
        for (int i = 0; i < (points.size()-1); i++){
            cv::Point2f 2Dpoint;
            cv::KeyPoint::convert(points[i], 2Dpoint);
            imagePoints.push_back(2Dpoint);
        }
        std::vector<cv::Point3f> modelPoints;
        modelPoints.push_back(cv::Point3f(/*help me*/));
        cv::Mat rotationMatrix translationMatrix;
        cv::solvePnPRansac(modelPoints, imagePoints, cameraMatrix, distortionMatrix, rotationMatrix, translationMatrix);
        
    }



    
}