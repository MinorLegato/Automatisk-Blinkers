#include "../lib/common.cc"
#include "../lib/klass.cc"
#include "../lib/matToLines.cc"
#include "../lib/image_proc.cc"
#include "../lib/crc32.h"
#include "canlib.h"
#include "net.h"

#include "../controller/controller.c"

#include <thread>

#include <iostream>

#define ARRAY_COUNT(array) (sizeof (array) / sizeof (array[0]))

#if 0
struct Controller
{
	int8_t 	thrust;
	int8_t 	steering;
	int8_t 	blink;
};
#endif

float   pos;
int     blink;

static Controller controller = {0};

static void ControllerThread(void)
{
	Can 		can         = {0};
	Server 		server      = {0};

    //puts("can");
	canInit(&can);
    //puts("net");
	netServerInit(&server, 8888);

	while (1) {
        //puts("net recv");
        ControllerPackage cp = {0};

		netServerRecv(&server, &cp, sizeof (ControllerPackage));

        uint32_t code = CRC32Code(&cp.controller, sizeof (Controller));

        if (cp.crc32 == code) {
            controller = cp.controller;

            controller.blink = blink;

            //printf("s %d\n", controller.steering);
            //printf("b %d\n", controller.blink);

            //puts("can send");
            canSend(&can, 0x7DF, &controller, sizeof (Controller));
        }
    }
}

int main(void)
{
    std::thread controller_thread(ControllerThread);

    cv::VideoCapture cap(0);

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  320);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);

    //ImageProcInit();

    InterPosList klass;

    cv::Mat frame;

    while (cv::waitKey(16) != 27) {
        cap >> frame;

        InterPos state = ImageProcUpdate(frame);
        klass.push(state);

        //blink = klass.analyze();
        //blink = klass.posAvg();
        
        //printf("blink %d\n", klass.blink);
        //printf("pos %.2f\n", klass.pos);
        //printf("pos %d\n", klass.type);

        //if (state.type & ROAD_UP)    puts("found up");
        //if (state.type & ROAD_LEFT)  puts("found left");
        //if (state.type & ROAD_RIGHT) puts("found right");

        ImageProcRender();
    }

#if 0
    Tilemap map;

    cv::namedWindow("tilemap",  cv::WINDOW_NORMAL);
    cv::namedWindow("capture",  cv::WINDOW_NORMAL);

    cv::Mat capture;

    int dialate_count = 0;

	InterPosList klassification;

    while (true) {
        int key = cv::waitKey(16);

        if (key == 27)  break;
        if (key == '1') dialate_count--;
        if (key == '2') dialate_count++;

        dialate_count = CLAMP(dialate_count, 0, 10);

        cap >> capture;
        
        {
            MatToEdge(capture);
        }

        TilemapResize(&map, capture.cols, capture.rows, 16);
        TilemapClear(&map);

      	// printf("%d %d\n", map.width, map.height);

        {
            TilemapFillEdges(&map, capture.ptr(), capture.cols, capture.rows);

            for (int i = 0; i < dialate_count; ++i) {
                TilemapDialate(&map);
            }
        }
    
        {
            TilemapFloodFill(&map,&map, map.width / 2, map.height - 1, TILE_ROAD);
        }

        {
            TilemapDrawRoadCenter(&map, &map,1);
        }

        RoadState state = GetRoadState(&map); // type
        pos   = GetRoadPosition(&map,state); //pos

		InterPos typePos;
		typePos.type = state;
		typePos.pos = pos;
		
        {
            klassification.push(typePos);
            klassification.analyze();
        }

        system("clear");

		printf("klass:  Blink %d\n" ,klassification.blink);
		printf("klass: PosAvg %.2f\n" ,klassification.pos);
        std::cout << klassification.pos << "\n";
        printf("klass: type %d\n", klassification.type);
		printf("klass:  PosDifAvg %.2f\n" ,klassification.posDifAvg);


        printf("position: %.2f\n", pos);
        printf("position: %d\n", typePos.type);
        

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
                    }
                }
            }

            cv::resizeWindow("tilemap", 0.5f * map.width * map.cell_size, 0.5f * map.height * map.cell_size);
            cv::imshow("tilemap", tilemap);
        }


        cv::resizeWindow("capture", 0.5f * capture.cols, 0.5f * capture.rows);
        cv::imshow("capture", capture);
    }
#endif

	return 0;
}


#if 0
	Can 		can         = {0};
	Server 		server      = {0};
	Controller 	controller  = {0};

    puts("can");
	canInit(&can);
    puts("net");
	netServerInit(&server, 8888);

	while (1) {
        puts("net recv");
		netServerRecv(&server, &controller, sizeof (Controller));

		printf("t %d\n", controller.thrust);
		printf("s %d\n", controller.steering);
		printf("b %d\n", controller.blink);

        puts("can send");
		canSend(&can, 0x7DF, &controller, sizeof (Controller));
	}
#endif



