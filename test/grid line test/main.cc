#include "../lib/common.cc"
#include "../lib/matToLines.cc"

int main(void) {
    cv::Mat capture = cv::imread("../testPics/3crossingtest2.png");

    std::vector<cv::Vec4i> line_vector;
    line_vector.reserve(8 * 2048);

    LineGrid grid;
    initGrid(&grid, 32);

    matToLines(capture, line_vector);

    cv::Mat lines = cv::Mat::zeros(capture.size(), capture.type());

    // find lines:
    {

        resizeGrid(&grid, capture.cols, capture.rows);
        clearGrid(&grid);

        for (auto line : line_vector) {
            addLineToGrid(&grid, line);
        }
    }

    {
        system("cls");

        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                const LineCell *cell = grid.getCell(x, y);

                for (int i = 0; i < cell->count; ++i) {
                    const f32 *line = cell->array[i];
                    cv::line(lines, { (int)line[0], (int)line[1] }, { (int)line[2], (int)line[3] }, { 255 }, 2);
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
