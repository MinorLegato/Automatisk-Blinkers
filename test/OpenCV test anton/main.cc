#include "common.cc"

int main(void) {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) { return -1; }

    cv::Mat capture, edges;

    std::vector<cv::Vec4i>  line_vector;
    line_vector.reserve(8 * 2048);

    LineGrid grid;
    initGrid(&grid, 32);

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        cv::Mat lines = cv::Mat::zeros(capture.size(), capture.type());

        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(capture, edges, { 5, 5 }, 0);
        cv::Canny(edges, edges, 50, 150);

        // find lines:
        {
            line_vector.resize(0);

            resizeGrid(&grid, capture.cols, capture.rows);
            clearGrid(&grid);

            cv::HoughLinesP(edges, line_vector, 2, CV_PI / 180, 20, 32, 16);

            for (auto line : line_vector) {
                cv::line(lines, { line[0], line[1] }, { line[2], line[3] }, { 0, 100, 255 }, 2);
                addLineToGrid(&grid, line);
            }
        }

        {
            system("cls");

            printf("%d\n", (int)line_vector.size());

            for (int y = 0; y < grid.height; ++y) {
                for (int x = 0; x < grid.width; ++x) {
                    auto cell = grid.getCell(x, y);

                    if (cell->count > 0) {
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
