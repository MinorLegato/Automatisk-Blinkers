#include "common.cc"

int main(void) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) { return -1; }

    cv::Mat capture, edges, lines;

    std::vector<cv::Vec4i>  line_vector;
    line_vector.reserve(8 * 2048);

    // 640 480
    cv::Point points[] = {
        { 0,   480 },
        { 640, 480 },
        { 640 / 2, 480 / 2 }
    };

    LineGrid grid;

    grid.init(640, 480, 32);

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        lines.create(capture.size(), capture.type());
        lines.setTo(cv::Scalar(0, 0, 0));

        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(capture, edges, { 5, 5 }, 0);
        cv::Canny(edges, edges, 50, 150);

        // find lines:
        {
            line_vector.resize(0);
            grid.clear();

            cv::HoughLinesP(edges, line_vector, 2, CV_PI / 180, 20, 32, 16);
            
            for (auto l : line_vector) {
                cv::line(lines, { l[0], l[1] }, { l[2], l[3] }, { 0, 100, 255 }, 2);

                grid.addLine(lineCreate(l[0], l[1], l[2], l[3]));
            }
        }

        {
            for (int y = 0; y < grid.height; ++y) {
                for (int x = 0; x < grid.width; ++x) {
                    auto cell = grid.get(x, y);

                    if (cell->size() > 0) {
                        putchar('#');
                    } else {
                        putchar('.');
                    }
                }

                putchar('\n');
            }

            putchar('\n');
        }

        cv::imshow("capture", capture);
        cv::imshow("edges", edges);
        cv::imshow("lines", lines);
    }
}

