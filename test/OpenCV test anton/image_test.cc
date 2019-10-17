
// d1 - 11:00 - 17:00
// d2 - 12:00 

int main(void) {
    cv::Mat edges, lines;

    edges = cv::imread("road5.png");

    lines.create(edges.size(), edges.type());
    lines.setTo(cv::Scalar(0, 0, 0));

    cv::cvtColor(edges, edges, cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
    cv::Canny(edges, edges, 25, 100);

    std::vector<cv::Vec4i> line_vector;

    cv::HoughLinesP(edges, line_vector, 1, CV_PI / 180, 4, 4, 4);
    
    for (auto l : line_vector) {
        cv::line(lines, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1);
    }

    cv::namedWindow("lines", cv::WINDOW_NORMAL);
    cv::namedWindow("edges", cv::WINDOW_NORMAL);
    cv::imshow("lines", lines);
    cv::imshow("edges", edges);

    cv::waitKey();
}

