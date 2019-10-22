#include "opencv2/opencv.hpp"

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <array>

int main(void) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) { return -1; }

    cv::namedWindow("capture");
    cv::namedWindow("edges");
    cv::namedWindow("mask");
    cv::namedWindow("lines");

    cv::Mat capture, edges, lines;

    std::vector<cv::Vec4i>  line_vector;
    line_vector.reserve(8 * 2048);

<<<<<<< Updated upstream
    // 640 480
    cv::Point points[] = {
        cv::Point(0,   480),
        cv::Point(640, 480),
        cv::Point(640 / 2, 480 / 2)
    };
=======
    Mat src = Mat::zeros( src.rows,src.cols, CV_8UC1 );

    
>>>>>>> Stashed changes

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        lines.create(capture.size(), capture.type());
        lines.setTo(cv::Scalar(0, 0, 0));

        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);

        cv::GaussianBlur(capture, edges, { 5, 5 }, 0);

        cv::Canny(edges, edges, 50, 150);

        cv::Mat mask = cv::Mat::zeros(edges.size(), CV_8UC1);

        cv::fillConvexPoly(mask, points, std::size(points), cv::Scalar(255));

        cv::Mat result;
        cv::bitwise_and(edges, mask, result);

        // find lines:
#if 1
        {
            line_vector.resize(0);
            cv::HoughLinesP(result, line_vector, 2, CV_PI / 180, 20, 32, 16); // alot of detail!!
            
            for (auto l : line_vector) {
                cv::line(lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 100, 255), 2);
            }
        }
#endif

        cv::imshow("capture", capture);
        cv::imshow("edges", edges);
        cv::imshow("mask", result);
        cv::imshow("lines", lines);
    }
}

