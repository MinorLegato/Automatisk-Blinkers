#include "../lib/common.cc"
#include "../lib/matToLines.cc"

#include "time.h"

int main(void) {
    cv::VideoCapture cap("../testPics/test_video.mp4");
    //cv::VideoCapture cap(0);

    Tilemap map(8);

    cv::namedWindow("capture", cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap", cv::WINDOW_NORMAL);

    cv::Mat capture;
    while (cv::waitKey(16) != 27) {
        system("cls");

        cap >> capture;

        cv::pyrDown(capture, capture, cv::Size { capture.cols / 2, capture.rows / 2 });

        if (1) {
            clock_t start = clock();
            
            cv::flip(capture, capture, 0);
            cv::flip(capture, capture, 1);

            clock_t end = clock();

            printf("flip ms: %d\n", (int)(end - start));
        }

        {
            clock_t start = clock();
            matToEdge(capture);
            clock_t end = clock();

            printf("MatToLines ms: %d\n", (int)(end - start));
        }

        map.resize(capture.cols, capture.rows);
        map.clear();

        std::cout << map.width << ' ' << map.height << '\n';

        {
            clock_t start = clock();
            tilemapFill(&map, capture.ptr(), capture.cols, capture.rows);
            clock_t end = clock();

            printf("TilemapFill ms: %d\n", (int)(end - start));
        }

        {
            clock_t start = clock();
            floodFill(&map, map.width / 2, map.height - 2, TILE_ROAD);
            clock_t end = clock();

            printf("FloodFill ms: %d\n", (int)(end - start));
        }

        RoadState   state   = getRoadState(&map);
        float       pos     = getRoadPosition(&map);

        std::cout << "position: " << pos << '\n';
        if (state & ROAD_UP)    std::cout << "found up\n";
        if (state & ROAD_LEFT)  std::cout << "found left\n";
        if (state & ROAD_RIGHT) std::cout << "found right\n";
    
        {
            cv::Mat tilemap = cv::Mat::zeros(map.height, map.width, CV_8UC3);

            clock_t start = clock();

            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int         tile    = map.get(x, y);
                    cv::Vec3b&  pixel   = tilemap.at<cv::Vec3b>(y, x);

                    switch (tile) {
                        case TILE_EDGE:
                            pixel[0] = 255;
                            pixel[1] = 0;
                            pixel[2] = 0;
                            break;
                        case TILE_ROAD:
                            pixel[0] = 0;
                            pixel[1] = 255;
                            pixel[2] = 0;
                            break;
                    }
                }
            }

            clock_t end = clock();

            printf("AsciiMap ms: %d\n", (int)(end - start));

            cv::resizeWindow("tilemap", map.width * 8, map.height * 8);
            cv::imshow("tilemap", tilemap);
        }

        cv::resizeWindow("capture", capture.cols, capture.rows);
        cv::imshow("capture", capture);
    }

    cv::waitKey(0);
}

