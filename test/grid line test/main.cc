#include "../lib/common.cc"
#include "../lib/matToLines.cc"

int main(void) {
    cv::VideoCapture cap("../testPics/test_video.mp4");

    std::vector<cv::Vec4i> line_vector;
    line_vector.reserve(8 * 2048);

    Tilemap map(16);

    cv::Mat capture;
    while (cv::waitKey(16) != 27) {
        system("cls");

        cap >> capture;

        puts("scale down");
        cv::pyrDown(capture, capture, cv::Size { capture.cols / 2, capture.rows / 2 });

        puts("scale flip");
        cv::flip(capture, capture, 0);
        cv::flip(capture, capture, 1);

        puts("do filters");
        matToLines(capture, line_vector);

        puts("resize map");
        map.resize(capture.cols, capture.rows);
        map.clear();

        std::cout << map.width << ' ' << map.height << '\n';

        puts("tilemap fill lines");
        tilemapFill(&map, capture.ptr(), capture.cols, capture.rows);

        puts("tilemap flood fill road");
        floodFill(&map, map.width / 2, map.height - 2, TILE_ROAD);

        puts("get road state");
        RoadState state = getRoadState(&map);

        if (state & ROAD_UP)    std::cout << "found up\n";
        if (state & ROAD_LEFT)  std::cout << "found left\n";
        if (state & ROAD_RIGHT) std::cout << "found right\n";

        puts("render ascii map");
        if (1) {
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

        puts("render capture map");
        cv::imshow("capture", capture);
    }

    cv::waitKey(0);
}

