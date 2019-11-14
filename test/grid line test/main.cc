#include "../lib/common.cc"
#include "../lib/matToLines.cc"

int main(void) {
    cv::VideoCapture cap("../testPics/test_video.mp4");

    std::vector<cv::Vec4i> line_vector;
    line_vector.reserve(8 * 2048);

    Tilemap map(32);

    cv::Mat capture;
    while (cv::waitKey(16) != 27) {
        cap >> capture;


        cv::pyrDown(capture, capture, cv::Size { capture.cols / 2, capture.rows / 2 });

        cv::flip(capture, capture, 0);
        cv::flip(capture, capture, 1);

        matToLines(capture, line_vector);

        cv::Mat lines = cv::Mat::zeros(capture.size(), capture.type());

        // find lines:
        {
            map.resize(capture.cols, capture.rows);
            map.clear();

            for (auto line : line_vector) {
                map.addLine(line);
                cv::line(lines, { line[0], line[1] }, { line[2], line[3] }, { 255 }, 2);
            }
        }

        floodFill(&map, map.width / 2, map.height - 2, TILE_ROAD);

        RoadState state = getRoadState(&map);

        system("cls");
        if (state & ROAD_UP)    std::cout << "found up\n";
        if (state & ROAD_LEFT)  std::cout << "found left\n";
        if (state & ROAD_RIGHT) std::cout << "found right\n";

        {
            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int tile = map.get(x, y);

                    switch (tile) {
                        case 0:
                            putchar('.');
                            break;
                        case 1:
                            putchar('#');
                            break;
                        case 2:
                            putchar('-');
                            break;

                    }
                }

                putchar('\n');
            }

            putchar('\n');
        }

        cv::imshow("capture", capture);
        cv::imshow("lines", lines);
    }

    cv::waitKey(0);
}

