#include <iostream>
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

std::vector<cv::Point> FindTarget(std::vector<std::vector<cv::Point>> matrix){
    std::vector<cv::Point> vec;
    for (size_t i = 0; i < matrix.size(); i++){ // converts vector of vector of points to vector of points
        for (size_t j = 0; j < matrix[i].size(); j++){
            vec.push_back(matrix[i][j]);
        }
    }
    for (size_t i = 0; i < vec.size(); i++){ // find the first corner, which should have a reasonable positive slope
        if (vec[i+1].y - vec[i].y > 25){ // TODO: find a reasonable tolerance 
            break;
        } else {
            vec.erase(vec.begin() + i);
            i--;
        }
    }
    for (size_t i = 4; i < vec.size(); i++){ // skips the next 3 points 
        if (vec[i-1].y - vec[i].y > 70) { // TODO: find a reasonable tolerance
            break;
        } else {
            vec.erase(vec.begin() + i);
            i--;
        }
    }
    for (size_t i = 8; i < vec.size(); i++){ // skips the next 3 points and deletes the rest
        vec.erase(vec.begin() + i);
        i--;
    } 
    return vec;
}

int main() {
    std::cout << "Start" << std::endl;
    GripPipeline pipe;
    cs::UsbCamera camera = frc::CameraServer::GetInstance()->StartAutomaticCapture(); // creates camera named "USB Camera 0"
    cs::CvSink cvsink = frc::CameraServer::GetInstance()->GetVideo();
    cs::MjpegServer server{"Server", 1182};
    uint16_t socketPort = 5001;
    uint32_t ip = GetIP(10, 35, 12, 2);
    UdpSocket socket;
    cv::Mat input;

    socket.bind(socketPort);
    camera.SetExposureManual(0);
    server.SetSource(camera);

    
    // Camera internals
    std::cout << "GRABBING" << std::endl;
    cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
    double focalLengthx = input.cols;
    double focalLengthy = input.rows;
    cv::Point2d center = cv::Point2d(input.cols/2, input.rows/2);
    cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << focalLengthx, 0, center.x, 0, focalLengthy, center.y, 0, 0, 1);
    cv::Mat distortionMatrix = cv::Mat::zeros(4,1,cv::DataType<double>::type); // no distortion assumed
    std::vector<cv::Point3f> modelPoints;
    modelPoints.push_back(cv::Point3f(0, 12.6965, 0.5)); // bottom-right most corner, clockwise        modelPoints.push_back(cv::Point3f(0, 10.76, 0));
    modelPoints.push_back(cv::Point3f(0, 0, 0));
    modelPoints.push_back(cv::Point3f(0, -1.9635, 0.5));
    modelPoints.push_back(cv::Point3f(0, -0.5565, 5.8241));
    modelPoints.push_back(cv::Point3f(0, 1.38, 5.3241));
    modelPoints.push_back(cv::Point3f(0, 9.38, 5.3241)); 
    modelPoints.push_back(cv::Point3f(0, 11.3165, 5.8241));    
    // Main loop
    while (1){
        std::cout << "ENTERED WHILE LOOP" << std::endl;
        cvsink.GrabFrameNoTimeout(input); // blocks until frame is avaliable
        pipe.Process(input);
        std::vector<std::vector<cv::Point>> output = *pipe.GetPolyDPOutput(); 
        std::cout << "PROCESSED" << std::endl;

        std::vector<cv::Point> imagePoints = FindTarget(output);
        
        if (imagePoints.size() != 8){
            continue;
        }

        cv::Mat rotationMatrix, translationMatrix;
        cv::solvePnPRansac(modelPoints, imagePoints, cameraMatrix, distortionMatrix, rotationMatrix, translationMatrix);
        std::cout << "SOLVED" << std::endl;

        PnP pnp;
        pnp.rotation = rotationMatrix, pnp.translation = translationMatrix;
        socket.send(&pnp, sizeof(pnp), ip, socketPort);
        std::cout << "SENT" << std::endl;
    }  
} 
