#include "../lib/common.cc"
#include "../lib/matToLines.cc"

#include <time.h>

#include <vector>

#if 0

int main(void)
{
    cv::Mat capture = cv::imread("../testPics/real1.jpg");

#if 1
    cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
    cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
    cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
#endif

    Tilemap map = {0};

    cv::namedWindow("capture", cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap", cv::WINDOW_NORMAL);

    MatToEdge(capture);

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
                    case TILE_LANE_CENTER:  pixel = { 150, 100, 50 };    break;
                }
            }
        }

        cv::resizeWindow("tilemap", map.width * map.cell_size, map.height * map.cell_size);
        cv::imshow("tilemap", tilemap);
    }

    cv::resizeWindow("capture", capture.cols, capture.rows);
    cv::imshow("capture", capture);

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

    cv::namedWindow("capture",      cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap",      cv::WINDOW_NORMAL);
    cv::namedWindow("hough_lines",  cv::WINDOW_NORMAL);

    cv::Mat capture;

    std::vector<cv::Vec4i> hough_lines;
    hough_lines.reserve(1024 * 1024);

    while (true) {
        int key = cv::waitKey(16);

        if (key == 27) break;

        system("cls");

        cap >> capture;

        if (1) {
            cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
            cv::pyrDown(capture, capture, { capture.cols / 2, capture.rows / 2 });
            
            cv::flip(capture, capture, 0);
            cv::flip(capture, capture, 1);
        }

        MatToEdge(capture);

        cv::imshow("canny", capture);

        TilemapResize(&map, capture.cols, capture.rows, 8);
        TilemapClear(&map);

        TilemapFillEdges(&map, capture.ptr(), capture.cols, capture.rows);

        TilemapFloodFillRoad(&map, &map, map.width / 2, map.height - 1);
        //TilemapDialate();

        // road edge masking
        {
            cv::Mat and_mat = cv::Mat::zeros(capture.rows, capture.cols, CV_8UC1);

            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int tile = TilemapGet(&map, x, y);

                    if (tile == TILE_ROAD_EDGE) {
                        cv::Point a = { (int)(map.cell_size * x), (int)(map.cell_size * y) };
                        cv::Point b = a + cv::Point(map.cell_size, map.cell_size);

                        cv::rectangle(and_mat, a, b, cv::Scalar(255), -1);
                    }
                }
            }

            cv::bitwise_and(capture, and_mat, capture);

            cv::imshow("and", and_mat);
        }

        TilemapDrawRoadCenter(&map, &map, 0);

        RoadState state = TilemapGetRoadState(&map);
        float     pos   = TilemapGetRoadPosition(&map, state);

        if (state & ROAD_UP)    puts("found up");
        if (state & ROAD_LEFT)  puts("found left");
        if (state & ROAD_RIGHT) puts("found right");
    
        // get hough_lines:
        {
            clock_t start = clock();

            cv::Mat lines = cv::Mat::zeros(capture.rows, capture.cols, CV_8UC3);

	        cv::HoughLinesP(capture, hough_lines, 2, CV_PI / 90.0f, 20, 10, 40);

            for (int i = 0; i < hough_lines.size(); ++i) {
                auto line = hough_lines[i];

                cv::Point a = { line[0], line[1] };
                cv::Point b = { line[2], line[3] };

                cv::line(lines, a, b, cv::Scalar(255, 255, 255), 2);
            }

            cv::resizeWindow("hough_lines", lines.cols, lines.rows);
            cv::imshow("hough_lines", lines);

            clock_t end = clock();

            printf("%d\n", (int)(end - start));
        }

        {
            cv::Mat tilemap = cv::Mat::zeros(map.height, map.width, CV_8UC3);

            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int         tile    = TilemapGet(&map, x, y);
                    cv::Vec3b&  pixel   = tilemap.at<cv::Vec3b>(y, x);

                    switch (tile) {
                        case TILE_EDGE:         pixel = { 255,    0,   0 }; break;
                        case TILE_ROAD:         pixel = {   0,  255,   0 }; break;
                        case TILE_ROAD_EDGE:    pixel = {  25,  100,  50 }; break;
                        case TILE_CENTER:       pixel = {   0,  100, 255 }; break;
                        case TILE_LANE_CENTER:  pixel = { 150,   70,  50 }; break;
                    }
                }
            }

            cv::resizeWindow("tilemap", map.width * map.cell_size, map.height * map.cell_size);
            cv::imshow("tilemap", tilemap);
        }

        cv::resizeWindow("capture", capture.cols, capture.rows);
        cv::imshow("capture", capture);
    }
}

#endif
