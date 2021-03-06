#pragma once

/* Find lines from Matrix
src - input matrix
linesP - output vector of Vec4i lines
*/

void MatToLines(cv::Mat &src, std::vector<cv::Vec4i> &linesP) {
    linesP.resize(0);

	cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(src, src, { 5, 5 }, 0);
	cv::Canny(src, src, 50, 150);
	cv::HoughLinesP(src, linesP, 1, CV_PI / 180.0f, 20, 10, 100);
}

void MatToEdge(cv::Mat &dst, const cv::Mat &src, int a = 50, int b = 150) {
	cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(dst, dst, { 5, 5 }, 0);
	cv::Canny(dst, dst, a, b);
}

