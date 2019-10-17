
#include "opencv2/opencv.hpp"
#include <cstdlib>
#include <cstdint>

typedef float   f32;
typedef double  f64;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

typedef unsigned long long  usize;
typedef long long           isize;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

template <typename T>
inline constexpr T CLAMP(const T& n, const T& low, const T& hi) {
    if (n < low) { return low; }
    if (n > hi)  { return hi; }

    return n;
}

static uchar sample_point_grayscale(const uchar* image, int width, int height, int px, int py, int sample_rad) {
    int sx = CLAMP(px - sample_rad, 0, width  - 1);
    int ex = CLAMP(px + sample_rad, 0, width  - 1);
    int sy = CLAMP(py - sample_rad, 0, height - 1);
    int ey = CLAMP(py + sample_rad, 0, height - 1);

    int size   = (ex - sx) * (ey - sy);
    int sample = 0;

    for (int y = sy; y < ey; ++y) {
        for (int x = sx; x < ex; ++x) {
            sample += image[y * width + x];
        }
    }

    return (uchar)(sample / size);
}

static void apply_kernel(uchar* dest, const uchar* source, int width, int height, const f32* kernel, int kernel_width, int kernel_height) {
    int krw = kernel_width  / 2;
    int krh = kernel_height / 2;

    for (int iy = krh; iy < height - krh; ++iy) {
        for (int ix = krw; ix < width - krw; ++ix) {
            f32 result = 0.0f;

            for (int ky = 0; ky < kernel_height; ++ky) {
                for (int kx = 0; kx < kernel_width; ++kx) {
                    result += source[(iy - krh) * width + (ix - krw)] * kernel[ky * kernel_width + kx];
                }
            }

            dest[iy * width + ix] = result;
        }
    }
}

int main(void) {
    cv::VideoCapture cap("hood.mp4");

    if (!cap.isOpened()) { return -1; }

    cv::namedWindow("capture");
    cv::namedWindow("result");

    cv::Mat capture, result;

    const f32 kernel[9] = {
        1.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 1.0f
    };

    while (cv::waitKey(16) != 27) {
        cap >> capture;

        cv::cvtColor(capture, capture, cv::COLOR_BGR2GRAY);

        result.create(capture.size(), capture.type());

        int width   = capture.cols;
        int height  = capture.rows;

        uchar* pixels   = capture.ptr();
        uchar* dest     = result.ptr();

        apply_kernel(dest, pixels, width, height, kernel, 3, 3);

        cv::imshow("capture", capture);
        cv::imshow("result", result);
    }
}

