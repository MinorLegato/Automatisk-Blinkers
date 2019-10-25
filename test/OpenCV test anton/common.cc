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

inline v2 operator+(const v2& a, const v2& b) { return { a.x + b.x, a.y + b.y }; }
inline v2 operator-(const v2& a, const v2& b) { return { a.x - b.x, a.y - b.y }; }
inline v2 operator*(const v2& a, f32 s)       { return { a.x * s, a.y * s }; }
inline v2 operator*(f32 s, const v2& a)       { return { a.x * s, a.y * s }; }
inline v2 operator/(const v2& a, f32 s)       { return { a.x / s, a.y / s }; }

inline void operator+=(v2& a, const v2& b) {
    a.x += b.x;
    a.y += b.y;
}

inline void operator-=(v2& a, const v2& b) {
    a.x -= b.x;
    a.y -= b.y;
}

inline void operator*=(v2& a, f32 s) {
    a.x *= s;
    a.y *= s;
}

inline void operator/=(v2& a, f32 s) {
    a.x /= s;
    a.y /= s;
}

inline f32 dot(const v2& a, const v2& b) {
    return a.x * b.x + a.y * b.y;
}

inline f32 lenSq(const v2& a) {
    return dot(a, a);
}

inline f32 len(const v2& a) {
    return std::sqrt(dot(a, a));
}

inline f32 distSq(const v2& a, const v2& b) {
    return lenSq(b - a);
}

inline f32 dist(const v2& a, const v2& b) {
    return std::sqrt(distSq(a, b));
}

inline v2 norm(const v2& a) {
    return a / len(a);
}

struct Line {
    v2  a;
    v2  b;
};

inline Line lineCreate(const v2& a, const v2& b) {
    Line line;

    line.a = a;
    line.b = b;

    return line;
}

inline Line lineCreate(f32 x0, f32 y0, f32 x1, f32 y1) {
    Line line;

    line.a.x = x0;
    line.a.y = y0;
    line.b.x = x1;
    line.b.y = y1;

    return line;
}

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
    
    T&       operator[](int index)       { return array[index]; }
    const T& operator[](int index) const { return array[index]; }
    //
    int size() const { return len; }

    T*       begin()       { return array; }
    T*       end()         { return array + len; }
    const T* begin() const { return array; }
    const T* end()   const { return array + len; }
};

// ============================================ LINE GRID ============================================== //

using LineArray = StaticArray<Line, 64>;

struct GridLine {
    i32         width       = 0;
    i32         height      = 0;
    i32         cell_size   = 0;
    LineArray*  cells       = nullptr;
};

// methods:
void
init(GridLine *grid, int image_width, int image_height, int cell_size) {
    grid->width     = image_width  / cell_size;
    grid->height    = image_height / cell_size;
    grid->cell_size = cell_size;
    grid->cells     = new LineArray[grid->width * grid->height];
}

void
clear(GridLine *grid) {
    int size = grid->width * grid->height;

    for (int i = 0; i < size; ++i) {
        grid->cells[i].clear();
    }
}

const LineArray *
get(const GridLine *grid, int x, int y) {
    return &grid->cells[y * grid->width + x];
}

LineArray *
get(GridLine *grid, int x, int y) {
    return &grid->cells[y * grid->width + x];
}

void
addLine(GridLine *grid, const Line& line) {
    v2 a    = line.a / f32(grid->cell_size);
    v2 b    = line.b / f32(grid->cell_size);
    v2 iter = a;
    v2 dir  = 0.5f * norm(b - a);

    while (distSq(iter, b) > 1.0f) {
        auto cell = get(grid, iter.x, iter.y);

        cell->add(line);

        iter += dir;
    }
}

