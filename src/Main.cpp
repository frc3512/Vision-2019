#include <vector>
#include <cscore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GripPipeline.h"
#include "opencv2/opencv.hpp"
#include "UdpSocket.hpp"

uint32_t GetIP(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3){
	return (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
}

int main() {
    GripPipeLine pipe;
    cs::UsbCamera camera;
    uint16_t port = 5001;
    uint32_t ip = GetIP(10, 35, 12, 2);
    UdpSocket socket;

    socket.bind(port);

    while (/*help me*/){
        if (/*help me*/){
            continue;
        }
        cv::Mat input = /*help me*/
        pipe.Process(input);
        std::vector<cv::KeyPoint> points = pipe.findBlobsOutput;
        std::vector<cv::Point2f> image_points;
        for (int i = 0; i < (points.size()-1); i++){
            cv::Point2f 2Dpoint;
            cv::KeyPoint::convert(points[i], 2Dpoint);
            image_points.push_back(2Dpoint);
        }
        std::vector<cv::Point3f> model_points;
        model_points.push_back(cv::Point3f(/*help me*/));
        cv::solvePnP(/*help me*/);

    }



    
}