#include "../lib/common.cc"
#include "../lib/matToLines.cc"

#include <time.h>

#include <vector>
mage 
static std::vector<cv::Vec4i>  hough_lines;

static cv::Mat                 mat_edge;
static cv::Mat                 mat_lines;
static cv::Mat                 mat_and;
static cv::Mat                 mat_tiles;

static Tilemap                 map;

static void ImageProcInit(void)
{
    hough_lines.reserve(1028 * 512);

    cv::namedWindow("edge", cv::WINDOW_NORMAL);
    cv::namedWindow("tilemap", cv::WINDOW_NORMAL);
    cv::namedWindow("hough_lines", cv::WINDOW_NORMAL);
    cv::namedWindow("and", cv::WINDOW_NORMAL);
}

static void ImageProcRender(void)
{
    cv::imshow("hough_lines",   mat_lines);
    cv::imshow("tilemap",       mat_tiles);
    cv::imshow("edge",          mat_edge);
    cv::imshow("and",           mat_and);
}

static void ImageProcUpdate(const cv::Mat &frame)
{
    MatToEdge(mat_edge, frame);

    TilemapResize(&map, mat_edge.cols, mat_edge.rows, 8);
    TilemapClear(&map);

    TilemapFillEdges(&map, mat_edge.ptr(), mat_edge.cols, mat_edge.rows);

    TilemapFloodFillRoad(&map, &map, map.width / 2, map.height - 1);

    // road edge masking
    {
        mat_and = cv::Mat::zeros(mat_edge.rows, mat_edge.cols, CV_8UC1);

        for (int y = 0; y < map.height; ++y) {
            for (int x = 0; x < map.width; ++x) {
                int tile = TilemapGet(&map, x, y);

                if (tile == TILE_ROAD_EDGE) {
                    cv::Point a = { (int)(map.cell_size * x), (int)(map.cell_size * y) };
                    cv::Point b = a + cv::Point(map.cell_size, map.cell_size);

                    cv::rectangle(mat_and, a, b, cv::Scalar(255), -1);
                }
            }
        }

        cv::bitwise_and(mat_edge, mat_and, mat_edge);
    }

    TilemapDrawRoadCenter(&map, &map, 0);

    RoadState state = TilemapGetRoadState(&map);
    float     pos   = TilemapGetRoadPosition(&map, state);

    if (state & ROAD_UP)    puts("found up");
    if (state & ROAD_LEFT)  puts("found left");
    if (state & ROAD_RIGHT) puts("found right");

    // get hough_lines:
    {
        mat_lines = cv::Mat::zeros(mat_edge.rows, mat_edge.cols, CV_8UC3);

        cv::HoughLinesP(mat_edge, hough_lines, 2, CV_PI / 90.0f, 20, 10, 100);

        for (int i = 0; i < hough_lines.size(); ++i) {
            auto line = hough_lines[i];

            cv::Point a = { line[0], line[1] };
            cv::Point b = { line[2], line[3] };

            cv::line(mat_lines, a, b, cv::Scalar(255, 255, 255), 2);
        }
    }

    {
        mat_tiles = cv::Mat::zeros(map.height, map.width, CV_8UC3);

        for (int y = 0; y < map.height; ++y) {
            for (int x = 0; x < map.width; ++x) {
                int         tile    = TilemapGet(&map, x, y);
                cv::Vec3b&  pixel   = mat_tiles.at<cv::Vec3b>(y, x);

                switch (tile) {
                    case TILE_EDGE:         pixel = { 255,    0,   0 }; break;
                    case TILE_ROAD:         pixel = {   0,  255,   0 }; break;
                    case TILE_ROAD_EDGE:    pixel = {  25,  100,  50 }; break;
                    case TILE_CENTER:       pixel = {   0,  100, 255 }; break;
                    case TILE_LANE_CENTER:  pixel = { 150,   70,  50 }; break;
                }
            }
        }
    }
}

#if 1

int main(void)
{
    cv::Mat frame = cv::imread("../testPics/real1.jpg");

#if 1
    cv::pyrDown(frame, frame, { frame.cols / 2, frame.rows / 2 });
    cv::pyrDown(frame, frame, { frame.cols / 2, frame.rows / 2 });
    cv::pyrDown(frame, frame, { frame.cols / 2, frame.rows / 2 });
#endif

    ImageProcInit();
    ImageProcUpdate(frame);
    ImageProcRender();

    cv::namedWindow("frame", cv::WINDOW_NORMAL);
    cv::imshow("frame", frame);

    cv::waitKey(0);
}

#else

int main(void)
{
    cv::VideoCapture cap("../testPics/test_video1.mp4");
    //cv::VideoCapture cap(0);

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  320 * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240 * 2);

    cv::Mat frame;

    ImageProc::Init();

    while (true) {
        int key = cv::waitKey(1);

        if (key == 27) break;

        system("cls");

        cap >> frame;

        if (1) {
            cv::pyrDown(frame, frame, { frame.cols / 2, frame.rows / 2 });
            cv::pyrDown(frame, frame, { frame.cols / 2, frame.rows / 2 });
            
            cv::flip(frame, frame, 0);
            cv::flip(frame, frame, 1);
        }

        {
            clock_t start = clock();
            ImageProc::Update(frame);
            clock_t end = clock();

            printf("%d\n", (int)(end - start));
        }

        ImageProc::Render();

        cv::imshow("frame", frame);
    }
}

#endif
