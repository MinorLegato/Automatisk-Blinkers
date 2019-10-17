#include "opencv2/opencv.hpp"
#include <cstdlib>
#include <vector>
#include <algorithm>

// d1 - 11:00 - 17:00
// d2 - 12:00 - 18:00
// d3 - 12:00 -

int main(void) {
    cv::VideoCapture cap("hood.mp4");

    if (!cap.isOpened()) { return -1; }

    cap.set(cv::CAP_PROP_POS_FRAMES, 5000);

    cv::namedWindow("capture");
    cv::namedWindow("edges");
    cv::namedWindow("lines");

    cv::Mat capture, edges, lines;

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        lines.create(capture.size(), capture.type());
        lines.setTo(cv::Scalar(0, 0, 0));

        cv::cvtColor(capture, capture, cv::COLOR_RGB2GRAY);
        cv::bilateralFilter(capture, edges, 7, 50, 50);

        cv::Canny(edges, edges, 60, 120);

        std::vector<cv::Vec4i> line_vector;

        cv::HoughLinesP(edges, line_vector, 1, CV_PI / 180, 4, 4, 4);
        
        for (auto l : line_vector) {
            cv::line(lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("capture", capture);
        cv::imshow("edges", edges);
        cv::imshow("lines", lines);
    }
}

