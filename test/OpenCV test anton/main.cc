#include "opencv2/opencv.hpp"

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>

static uint8_t sample_point_grayscale(const cv::Mat& image, int point_x, int point_y, int sample_rad) {
    int sx = std::clamp(point_x - sample_rad, 0, image.cols - 1);
    int sy = std::clamp(point_y - sample_rad, 0, image.rows - 1);
    int ex = std::clamp(point_x + sample_rad, 0, image.cols - 1);
    int ey = std::clamp(point_y + sample_rad, 0, image.rows - 1);

    int size   = (ex - sx) * (ey - sy);
    int sample = 0;

    for (int y = sy; y < ey; ++y) {
        for (int x = sx; x < ex; ++x) {
            sample += image.at<uint8_t>(y, x);
        }
    }

    return uint8_t(sample / size);
}

int main(void) {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) { return -1; }

    //cap.set(cv::CAP_PROP_POS_FRAMES, 5000);

    cv::namedWindow("capture");
    cv::namedWindow("edges");
    //cv::namedWindow("lines");

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
        //cv::imshow("lines", lines);
    }
}

