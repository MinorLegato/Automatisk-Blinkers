#include "../lib/common.cc"
#include "../lib/matToLines.cc"

#include <iostream>

/* Find lines in video called "testFotage.mp4.
output result in "testFotage_lines.mp4"
*/
int main(void) {
	const char *name = "../testPics/testFotage.mp4" ;
	const char *outname = "../testPics/testFotage_lines.mp4";

	cv::VideoCapture cap(name);

	if (!cap.isOpened()) {
		std::cout << "ERROR OPENING VIDEO";
		return -1;
	}
	int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	cv::VideoWriter video(outname, cap.get(cv::CAP_PROP_FOURCC), cap.get(cv::CAP_PROP_FPS), cv::Size(frame_width,frame_height));
	while (1) {
		cv::Mat src;
		std::vector<cv::Vec4i> lines;
		cap >> src;

		if (src.empty()) {
			break;
		}

		matToLines(src, lines);
		for (cv::Vec4i l : lines)
		{
			line(src, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
		}
		video.write(src);
	}

	cap.release();
	video.release();

	return 0;
}
