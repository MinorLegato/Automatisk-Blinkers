#include <opencv2/opencv.hpp>

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>

int main(void) {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) { return -1; }

    cv::namedWindow("capture");
    cv::namedWindow("edges");

    cv::Mat capture, edges, lines;

    std::vector<cv::Vec4i>  line_vector;

    line_vector.reserve(8 * 2048);

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        lines.create(capture.size(), capture.type());
        lines.setTo(cv::Scalar(0, 0, 0));

        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);

        cv::medianBlur(capture, edges, 7);
        //cv::bilateralFilter(capture, edges, 7, 100, 100); // alot of detail

        cv::Canny(edges, edges, 50, 150);
        //cv::cornerHarris(edges, edges, 3, 7, 3.0f);

        // find lines:
#if 0
        {
            line_vector.resize(0);
            cv::HoughLinesP(edges, line_vector, 1, CV_PI / 180, 4, 4, 4); // alot of detail!!
            
            for (auto l : line_vector) {
                cv::line(lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 100, 255), 2);
            }
        }
#endif

        cv::imshow("capture", capture);
        cv::imshow("edges", edges);
    }
}

