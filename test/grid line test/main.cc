#include "../lib/common.cc"
#include "../lib/matToLines.cc"

#include "time.h"

int main(void)
{
    cv::VideoCapture cap("../testPics/test_video.mp4");
    //cv::VideoCapture cap(0);

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  320 * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240 * 2);

    Tilemap map(16);

    cv::namedWindow("capture", cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap", cv::WINDOW_NORMAL);

    std::vector<IntersecPlacement> placement(10);
    int index = 0;

    cv::Mat capture;

    int dialate_count = 0;

    while (true) {
        int key = cv::waitKey(16);

        if (key == 27) break;

        if (key == '1') dialate_count--;
        if (key == '2') dialate_count++;

        dialate_count = CLAMP(dialate_count, 0, 10);

        system("cls");

        cap >> capture;

        if (1) {
            cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });

            clock_t start = clock();
            
            cv::flip(capture, capture, 0);
            cv::flip(capture, capture, 1);

            clock_t end = clock();

            printf("flip ms: %d\n", (int)(end - start));
        }

        {
            clock_t start = clock();
            MatToEdge(capture);
            clock_t end = clock();

            printf("MatToLines ms: %d\n", (int)(end - start));
        }

        map.Resize(capture.cols, capture.rows);
        map.Clear();

        printf("%d %d\n", map.width, map.height);

        {
            clock_t start = clock();

            TilemapFill(&map, capture.ptr(), capture.cols, capture.rows);

            for (int i = 0; i < dialate_count; ++i) {
                TilemapDialate(&map);
            }

            clock_t end = clock();

            printf("TilemapFill ms: %d\n", (int)(end - start));
        }

        {
            clock_t start = clock();
            FloodFill(&map, map.width / 2, map.height - 1, TILE_ROAD);
            clock_t end = clock();

            printf("FloodFill ms: %d\n", (int)(end - start));
        }

        RoadState state = GetRoadState(&map);
        float     pos   = GetRoadPosition(&map);

        placement[index % 10] = { state, pos };

        int klass = Klass(placement);

        printf("klass %d\n", klass);

        printf("position: %.2f\n", pos);

        if (state & ROAD_UP)    puts("found up");
        if (state & ROAD_LEFT)  puts("found left");
        if (state & ROAD_RIGHT) puts("found right");
    
        {
            cv::Mat tilemap = cv::Mat::zeros(map.height, map.width, CV_8UC3);

            clock_t start = clock();

            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int         tile    = map.Get(x, y);
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

            cv::resizeWindow("tilemap", map.width * map.cell_size, map.height * map.cell_size);
            cv::imshow("tilemap", tilemap);
        }

        cv::resizeWindow("capture", capture.cols, capture.rows);
        cv::imshow("capture", capture);
    }

    cv::waitKey(0);
}

