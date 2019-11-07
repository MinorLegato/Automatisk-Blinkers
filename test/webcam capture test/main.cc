#include "../lib/common.cc"
#include "../lib/matToLines.cc"

int main(void) {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) { return -1; }

    cv::Mat capture;

    std::vector<cv::Vec4i> line_vector;
    line_vector.reserve(8 * 2048);

    LineGrid grid;
    initGrid(&grid, 32);

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        matToLines(capture, line_vector);

        cv::Mat lines = cv::Mat::zeros(capture.size(), capture.type());

        // find lines:
        {

            resizeGrid(&grid, capture.cols, capture.rows);
            clearGrid(&grid);

            for (auto line : line_vector) {
                cv::line(lines, { line[0], line[1] }, { line[2], line[3] }, { 255 }, 2);
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
        //cv::imshow("edges", edges);
        cv::imshow("lines", lines);
    }
}
