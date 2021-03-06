#include "../../lib/common.cc"
#include "../../lib/matToLines.cc"

#include <time.h>

#if 1

int main(void)
{
    cv::Mat capture = cv::imread("../testPics/3crossingtest1.png");

#if 1
    cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
    //cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
    //cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
#endif

    cv::imshow("normal", capture);

    Tilemap map = {0};

    {
        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);
        cv::imshow("gray", capture);

        cv::GaussianBlur(capture, capture, { 5, 5 }, 0);
        cv::imshow("blur", capture);

        cv::Canny(capture, capture, 50, 150);
        cv::imshow("edge", capture);
    }

    //MatToEdge(capture, capture);

    TilemapResize(&map, capture.cols, capture.rows, 8);

    TilemapClear(&map);

    printf("%d %d\n", map.width, map.height);

    TilemapFillEdges(&map, capture.ptr(), capture.cols, capture.rows);

    TilemapFloodFill(&map, &map, map.width / 2, map.height - 1, TILE_ROAD);

    RoadState state = TilemapGetRoadState(&map);

    float per = TilemapDrawRoadCenter(&map, &map);

    printf("center edge per: %f\n", per);

    if (per > 0.2f)
        state |= ROAD_TWO_LANES;

    float pos = TilemapGetRoadPosition(&map, state);

    printf("position: %.2f\n", pos);

    if (state & ROAD_UP)        puts("found up");
    if (state & ROAD_LEFT)      puts("found left");
    if (state & ROAD_RIGHT)     puts("found right");
    if (state & ROAD_TWO_LANES) puts("found two lanes");

    {
        cv::Mat tilemap = cv::Mat::zeros(map.height, map.width, CV_8UC3);

        for (int y = 0; y < map.height; ++y) {
            for (int x = 0; x < map.width; ++x) {
                int         tile    = TilemapGet(&map, x, y);
                cv::Vec3b&  pixel   = tilemap.at<cv::Vec3b>(y, x);

                switch (tile) {
                    case TILE_EDGE:         pixel = { 255, 0, 0 };      break;
                    case TILE_ROAD:         pixel = { 0, 255, 0 };      break;
                    case TILE_CENTER:       pixel = { 0, 100, 255 };    break;
                    case TILE_LANE_CENTER:  pixel = { 150, 100, 50 };   break;
                }
            }
        }

        cv::namedWindow("tilemap", cv::WINDOW_NORMAL);
        cv::resizeWindow("tilemap", map.width * map.cell_size, map.height * map.cell_size);
        cv::imshow("tilemap", tilemap);
    }

    cv::waitKey(0);
}

#else

int main(void)
{
    cv::VideoCapture cap("../testPics/test_video1.mp4");
    //cv::VideoCapture cap(0);

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  320 * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240 * 2);

    Tilemap map;

    cv::namedWindow("capture", cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap", cv::WINDOW_NORMAL);

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

        TilemapResize(&map, capture.cols, capture.rows, 16);
        TilemapClear(&map);

        printf("%d %d\n", map.width, map.height);

        {
            clock_t start = clock();

            TilemapFillEdges(&map, capture.ptr(), capture.cols, capture.rows);

            for (int i = 0; i < dialate_count; ++i) {
                TilemapDialate(&map);
            }

            clock_t end = clock();

            printf("TilemapFill ms: %d\n", (int)(end - start));
        }

        {
            clock_t start = clock();
            TilemapFloodFill(&map, &map, map.width / 2, map.height - 1, TILE_ROAD);
            clock_t end = clock();

            printf("FloodFill ms: %d\n", (int)(end - start));
        }

        TilemapDrawRoadCenter(&map, &map, 0);

        RoadState state = TilemapGetRoadState(&map);
        float     pos   = TilemapGetRoadPosition(&map, state);

        printf("position: %.2f\n", pos);

        if (state & ROAD_UP)    puts("found up");
        if (state & ROAD_LEFT)  puts("found left");
        if (state & ROAD_RIGHT) puts("found right");
    
        {
            cv::Mat tilemap = cv::Mat::zeros(map.height, map.width, CV_8UC3);

            clock_t start = clock();

            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int         tile    = TilemapGet(&map, x, y);
                    cv::Vec3b&  pixel   = tilemap.at<cv::Vec3b>(y, x);

                    switch (tile) {
                        case TILE_EDGE:
                            pixel = { 255, 0, 0 };
                            break;
                        case TILE_ROAD:
                            pixel = { 0, 255, 0 };
                            break;
                        case TILE_CENTER:
                            pixel = { 0, 100, 255 };
                            break;
                        case TILE_LANE_CENTER:
                            pixel = { 150, 70, 50 };
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
#endif
