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

void *operator new    (size_t size)       { return std::malloc(size); }
void  operator delete (void *ptr)         { return std::free(ptr); }
void  operator delete (void *ptr, size_t) { return std::free(ptr); }

template <typename T>
static T *allocate(i32 count = 1) {
    return (T *)malloc(count * sizeof (T));
}

template <typename T>
static T *reallocate(T *ptr, i32 count) {
    return (T *)realloc(ptr, sizeof (T) * count);
}

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
    i32 len;
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

struct Grid {
    using Cell = StaticArray<Line, 64>;
    // data:
    i32     width;
    i32     height;
    Cell*   cells;

    // methods:
    Grid(int width, int height);
    ~Grid();

    void clear();
    void addLine(const Line &line);
};

inline Grid::Grid(int width, int height) {
    this->width  = width;
    this->height = height;
    this->cells  = new Cell[this->width * this->height];
}

inline Grid::~Grid() {
    delete[] this->cells;
}

inline void Grid::clear() {
    int size = this->width * this->height;

    for (int i = 0; i < size; ++i) {
        cells[i].clear();
    }
}

inline void Grid::addLine(const Line &line) {
    auto dir  = norm(line.b - line.a);
    auto iter = line.a;

    while (int(iter.x) != int(line.b.x) && int(iter.y) != int(line.b.x)) {
        auto cell = &cells[int(iter.y) * width + int(iter.x)];

        cell->add(line);

        iter += dir;
    }
}

