#include "opencv2/opencv.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdint>

#include <vector>
#include <algorithm>
#include <iostream>
#include <array>
#include <cmath>

using f32 = float;
using f64 = double;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// =============================================== MATH ================================================= //

struct v2 {
    f32   x;
    f32   y;
};

inline v2 operator+(const v2 &a, const v2 &b) {
    return { a.x + b.x, a.y + b.y };
}

inline v2 operator-(const v2 &a, const v2 &b) {
    return { a.x - b.x, a.y - b.y };
}

inline v2 operator*(const v2 &a, f32 s) {
    return { a.x * s, a.y * s };
}

inline v2 operator*(f32 s, const v2 &a) {
    return { a.x * s, a.y * s };
}

inline v2 operator/(const v2 &a, f32 s) {
    return { a.x / s, a.y / s };
}

inline void operator+=(v2 &a, const v2 &b) {
    a.x += b.x;
    a.y += b.y;
}

inline void operator-=(v2 &a, const v2 &b) {
    a.x -= b.x;
    a.y -= b.y;
}

inline void operator*=(v2 &a, f32 s) {
    a.x *= s;
    a.y *= s;
}

inline void operator/=(v2 &a, f32 s) {
    a.x /= s;
    a.y /= s;
}

inline f32 dot(const v2 &a, const v2 &b) {
    return a.x * b.x + a.y * b.y;
}

inline f32 lenSq(const v2 &a) {
    return dot(a, a);
}

inline f32 len(const v2 &a) {
    return std::sqrt(dot(a, a));
}

inline v2 norm(const v2 &a) {
    return a / len(a);
}

struct Line {
    v2  a;
    v2  b;
};

// ============================================== ARRAY ================================================ //

template <typename T, i32 N>
struct StaticArray {
    // data:
    i32 len = 0;
    T   array[N];

    // methods:
    void add(const T &e) { array[len++] = e; }
    void rem(int index)  { array[index] = array[--len]; }

    void clear() { len = 0; }
    
    T &       operator[](int index)       { return array[index]; }
    const T & operator[](int index) const { return array[index]; }
    //
    int size() const { return len; }

    T *       begin()       { return array; }
    T *       end()         { return array + len; }
    const T * begin() const { return array; }
    const T * end()   const { return array + len; }
};

// ============================================ LINE GRID ============================================== //

using LineArray = StaticArray<Line, 64>;

struct LineGrid {
    // data:
    i32         width       = 0;
    i32         height      = 0;
    i32         cell_size   = 0;
    LineArray*  cells      = nullptr;

    // methods:
    void init(int width, int height);
    void clear();
    void addLine(const Line& line);
};

void LineGrid::init(int w, int h) {
    width  = w;
    height = h;
    cells  = new LineArray[width * height];
}

inline void LineGrid::clear() {
    int size = this->width * this->height;

    for (int i = 0; i < size; ++i)
        cells[i].clear();
}

inline void LineGrid::addLine(const Line& line) {
    v2 dir  = norm(line.b - line.a);
    v2 iter = line.a;

    while (int(iter.x) != int(line.b.x) && int(iter.y) != int(line.b.x)) {
        LineArray* cell = &cells[int(iter.y) * width + int(iter.x)];

        cell->add(line);

        iter += dir;
    }
}


