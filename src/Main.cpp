#include <vector>
#include "cscore.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cameraserver/CameraServer.h"

#include "GripPipeline.h"
#include "opencv2/opencv.hpp"
#include "UdpSocket.hpp"

struct PnP {
    cv::Mat rotation;
    cv::Mat translation;
};

uint32_t GetIP(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3){
	return (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
}

int main() {
    GripPipeline pipe;
    cs::UsbCamera camera = frc::CameraServer::GetInstance()->StartAutomaticCapture(); // creates camera named "USB Camera 0"
    cs::CvSink cvsink = frc::CameraServer::GetInstance()->GetVideo();

    uint16_t socketPort = 5001;
    uint32_t ip = GetIP(10, 35, 12, 2);
    UdpSocket socket;
    cv::Mat input;

    // TODO: change camera to dark
    socket.bind(socketPort);

    // Camera internals
    cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
    double focalLengthx = input.cols;
    double focalLengthy = input.rows;
    cv::Point2d center = cv::Point2d(input.cols/2, input.rows/2);
    cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << focalLengthx, 0, center.x, 0, focalLengthy, center.y, 0, 0, 1);
    cv::Mat distortionMatrix = cv::Mat::zeros(4,1,cv::DataType<double>::type); // no distortion assumed

    // Main loop
    while (1){
        cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
        pipe.Process(input);
        std::vector<std::vector<cv::Point>> imagePoints = *pipe.GetPolyDPOutput(); 

        /*int count = 0;
        double averageX = 0.0, averageY = 0.0
        for (size_t i = 0; i < output.size(); i++){
            for (size_t j = 0; j < output[i].size(); j++){
                averageX += output[i][j].x;
                averageY += output[i][j].y;
                count++;
        }
        average*/

        std::vector<cv::Point3f> modelPoints;
        modelPoints.push_back(cv::Point3f(0, 9.38, 5.3241)); // right side tape's leftmost corner
        modelPoints.push_back(cv::Point3f(0, 11.3165, 5.8241));
        modelPoints.push_back(cv::Point3f(0, 12.6965, 0.5));
        modelPoints.push_back(cv::Point3f(0, 10.76, 0));
        modelPoints.push_back(cv::Point3f(0, 0, 0));
        modelPoints.push_back(cv::Point3f(0, -1.9635, 0.5));
        modelPoints.push_back(cv::Point3f(0, -0.5565, 5.8241));
        modelPoints.push_back(cv::Point3f(0, 1.38, 5.3241));
        cv::Mat rotationMatrix, translationMatrix;
        cv::solvePnPRansac(modelPoints, imagePoints, cameraMatrix, distortionMatrix, rotationMatrix, translationMatrix);
        
        PnP pnp;
        pnp.rotation = rotationMatrix, pnp.translation = translationMatrix;
        socket.send(&pnp, sizeof(pnp), ip, socketPort);
    }



    
}   
