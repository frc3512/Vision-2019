// Copyright (c) 2019 FRC Team 3512. All Rights Reserved.

#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <cameraserver/CameraServer.h>
#include <cscore.h>
#include <opencv2/opencv.hpp>

#include "GripPipeline.h"
#include "UdpSocket.hpp"
#include "subprocess.hpp"
#include "thirdparty/units.h"

struct PnP {
    cv::Mat rotation;
    cv::Mat translation;
};

uint32_t GetIP(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    return (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
}

std::vector<cv::Point2f> FindTarget(
    std::vector<std::vector<cv::Point>> matrix) {
    // Converts vector of vector of points to vector of points
    std::vector<cv::Point2f> vec;
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix[i].size(); j++) {
            vec.push_back(matrix[i][j]);
        }
    }

    // Finds an average center from all the points
    static double avgX = 0.0, avgY = 0.0;
    for (const auto& p : vec) {
        avgX += p.x;
        avgY += p.y;
    }
    avgX /= vec.size();
    avgY /= vec.size();

    // Sorts all points CCW from the postive x axis assuming (avgX, avgY) is the
    // origin
    std::sort(vec.begin(), vec.end(), [](const auto& lhs, const auto& rhs) {
        double lhsAngle = std::atan2(lhs.y - avgY, lhs.x - avgX);
        double rhsAngle = std::atan2(rhs.y - avgY, rhs.x - avgX);
        return lhsAngle < rhsAngle;
    });

    for (size_t i = 0; i < vec.size();
         i++) {  // find the first corner, which should have a reasonable
                 // positive slope
        if (vec[i + 1].y - vec[i].y > 1) {  // TODO: find a reasonable tolerance
            break;
        } else {
            vec.erase(vec.begin() + i);
            i--;
        }
    }

    for (size_t i = 4; i < vec.size(); i++) {  // skips the next 3 points
        if (vec[i].y - vec[i - 1].y >
            30) {  // TODO: find a reasonable tolerance
            break;
        } else {
            vec.erase(vec.begin() + i);
            i--;
        }
    }
    for (size_t i = 8; i < vec.size();
         i++) {  // skips the next 3 points and deletes the rest
        vec.erase(vec.begin() + i);
        i--;
    }
    /* for (size_t i = 0; i < vec.size(); i++) {
        std::cout << i << ": (" << vec[i].x << ", " << vec[i].y << ")"
                  << std::endl;
    } */
    return vec;
}

int main() {
    std::cout << "Start" << std::endl;
    grip::GripPipeline pipe;
    cs::UsbCamera cvCamera =
        frc::CameraServer::GetInstance()->StartAutomaticCapture();
    cs::UsbCamera driverCamera =
        frc::CameraServer::GetInstance()->StartAutomaticCapture();
    cs::CvSink cvSink = frc::CameraServer::GetInstance()->GetVideo();
    cs::MjpegServer server{"DriverView", 1128};
    uint16_t socketPort = 5001;
    uint32_t ip = GetIP(10, 35, 12, 2);
    UdpSocket socket;
    cv::Mat input;
    cs::CvSource CvSource;

    socket.bind(socketPort);
    // cvCamera.SetExposureManual(4);
    // cvCamera.SetExposureHoldCurrent();
    cvCamera.SetResolution(1080, 720);
    driverCamera.SetResolution(640, 480);
    cvCamera.GetProperty("auto_exposure_bias").Set(0);
    cvCamera.GetProperty("exposure_time_absolute").Set(0);
    cvCamera.SetBrightness(31);
    cvCamera.GetProperty("saturation").Set(65);
    cvCamera.GetProperty("sharpness").Set(100);

    server.SetSource(cvCamera);

    // Camera internals
    std::cout << "GRABBING" << std::endl;
    cvSink.GrabFrameNoTimeout(input);  // blocks until frame is avaliable
    // double focalLengthx = input.cols;
    // double focalLengthy = input.rows;
    // cv::Point2d center = cv::Point2d(input.cols / 2, input.rows / 2);
    cv::Mat cameraMatrix =
        (cv::Mat_<double>(3, 3) << 9.7251025460519838e+02, 0, 538, 0,
         9.7251025460519838e+02, 3.5650000000000000e+02, 0, 0, 1);
    cv::Mat distortionMatrix =
        (cv::Mat_<double>(5, 1) << 2.9666848459501894e-02,
         7.7071617830897565e-01, 0, 0, -2.7882844813877661e+00);
    std::vector<cv::Point3f> modelPoints;
    // top-right most corner, counter-clockwise (in meters)
    // (0, 0, 0) is approximately center of the target
    modelPoints.emplace_back(0, 0.1484074, 0.080289);
    modelPoints.emplace_back(0, 0.1, 0.06777);
    modelPoints.emplace_back(0, -0.1, 0.06777);
    modelPoints.emplace_back(0, -0.1484074, 0.080289);
    modelPoints.emplace_back(0, -0.1834606, -0.055251);
    modelPoints.emplace_back(0, -0.1350532, -0.06777);
    modelPoints.emplace_back(0, 0.1350532, -0.06777);
    modelPoints.emplace_back(0, 0.1834606, -0.055251);
    // Main loop
    while (1) {
        cvSink.GrabFrameNoTimeout(input);  // blocks until frame is avaliable
        pipe.Process(input);
        std::vector<std::vector<cv::Point>> output = *pipe.GetPolyDPOutput();

        std::vector<cv::Point2f> imagePoints = FindTarget(output);

        if (imagePoints.size() != 8) {
            std::cout << imagePoints.size() << " points, continuing"
                      << std::endl;
            continue;
        }

        // 3x1 vectors
        cv::Mat rVec, tVec;
        cv::solvePnPRansac(modelPoints, imagePoints, cameraMatrix,
                           distortionMatrix, rVec, tVec);

        double pose[] = {tVec.at<double>(0, 0), tVec.at<double>(1, 0),
                         rVec.at<double>(2, 0)};

        std::cout << "x: " << pose[0] << " y: " << pose[1]
                  << " Theta: " << pose[2] << std::endl;

        socket.send(&pose, sizeof(pose), ip, socketPort);
    }
}
