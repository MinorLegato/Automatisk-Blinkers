#include "../lib/common.cc"
#include "../lib/matToLines.cc"

int main(void) {
    cv::Mat capture = cv::imread("../testPics/3crossingtest2.png");

    std::vector<cv::Vec4i> line_vector;
    line_vector.reserve(8 * 2048);

    LineGrid grid(32);

    matToLines(capture, line_vector);

    cv::Mat lines = cv::Mat::zeros(capture.size(), capture.type());

    // find lines:
    {
        grid.resize(capture.cols, capture.rows);
        grid.clear();

        for (auto line : line_vector) {
            grid.addLine(line);
        }
    }

    getRoadState(grid);

    {
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                const LineCell *cell = grid.get(x, y);

                for (int i = 0; i < cell->count; ++i) {
                    const Line& line = cell->array[i];

                    cv::line(lines, { (int)line[0][0], (int)line[0][1] }, { (int)line[1][0], (int)line[1][1] }, { 255 }, 2);
                }

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
    cv::imshow("lines", lines);

    cv::waitKey(0);
}
