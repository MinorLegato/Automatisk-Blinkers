#include "../../lib/common.cc"
#include "../../lib/matToLines.cc"
#include "../../lib/klass.cc"
#include"../../lib/image_proc.cc"
#include <iostream>

#include "time.h"


int main(void)
{
    cv::VideoCapture cap("../testPics/test_video1.mp4");
    //cv::VideoCapture cap(0);

    cap.set(cv::CAP_PROP_FRAME_WIDTH,  320 * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240 * 2);

    cv::Mat frame;

    ImageProcInit();

    InterPosList klassList;

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
            InterPos state = ImageProcUpdate(frame);

            klassList.push(state);
            klassList.analyze();

            clock_t end = clock();

            printf("%d\n", (int)(end - start));
            printf("Klass blink %d\n", klassList.blink);
            printf("Klass type %d\n", klassList.type);
            printf("Klass pos %.2f\n", klassList.pos);
            printf("Pos In %.2f\n", state.pos);
        }

        ImageProcRender();

        cv::imshow("frame", frame);
    }
}
