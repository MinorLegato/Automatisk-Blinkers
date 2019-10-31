#include "opencv2/opencv.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdint>

#include <vector>
#include <algorithm>
#include <iostream>
#include <array>
#include <cmath>

// =============================================== MATH ================================================= //

struct v2 {
    float   x;
    float   y;
};

inline v2 operator+(const v2& a, const v2& b) { return { a.x + b.x, a.y + b.y }; }
inline v2 operator-(const v2& a, const v2& b) { return { a.x - b.x, a.y - b.y }; }
inline v2 operator*(const v2& a, float s)       { return { a.x * s, a.y * s }; }
inline v2 operator*(float s, const v2& a)       { return { a.x * s, a.y * s }; }
inline v2 operator/(const v2& a, float s)       { return { a.x / s, a.y / s }; }

inline void operator+=(v2& a, const v2& b) {
    a.x += b.x;
    a.y += b.y;
}

inline void operator-=(v2& a, const v2& b) {
    a.x -= b.x;
    a.y -= b.y;
}

inline void operator*=(v2& a, float s) {
    a.x *= s;
    a.y *= s;
}

inline void operator/=(v2& a, float s) {
    a.x /= s;
    a.y /= s;
}

inline float dot(const v2& a, const v2& b) {
    return a.x * b.x + a.y * b.y;
}

inline float lenSq(const v2& a) {
    return dot(a, a);
}

inline float len(const v2& a) {
    return std::sqrt(dot(a, a));
}

inline float distSq(const v2& a, const v2& b) {
    return lenSq(b - a);
}

inline float dist(const v2& a, const v2& b) {
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

inline Line lineCreate(float x0, float y0, float x1, float y1) {
    Line line;

    line.a.x = x0;
    line.a.y = y0;
    line.b.x = x1;
    line.b.y = y1;

    return line;
}

// ============================================== ARRAY ================================================ //

template <typename T, int N>
struct StaticArray {
    int len = 0;
    T   array[N];

    // methods:
    void add(const T &e) { array[len++] = e; }
    void rem(int index)  { array[index] = array[--len]; }

    void clear() { len = 0; }
    
    T&       operator[](int index)       { return array[index]; }
    const T& operator[](int index) const { return array[index]; }
    //
    int size() const { return len; }

    bool isFull() const {
        return len >= N;
    }

    T*       begin()       { return array; }
    T*       end()         { return array + len; }
    const T* begin() const { return array; }
    const T* end()   const { return array + len; }
};

// ============================================ LINE GRID ============================================== //

using LineArray = StaticArray<Line, 64>;

struct LineGrid {
    int         width;
    int         height;
    int         cell_size;
    LineArray   *cells;
    //
    LineArray *
    getCell(int x, int y) {
        return &cells[y * width + x];
    }

    const LineArray *
    getCell(int x, int y) const {
        return &cells[y * width + x];
    }
};

static void
initGrid(LineGrid *grid, int cell_size) {
    grid->cell_size     = cell_size;
    grid->cells         = nullptr;
}

static void
resizeGrid(LineGrid *grid, int image_width, int image_height) {
    grid->width     = image_width  / grid->cell_size;
    grid->height    = image_height / grid->cell_size;
    grid->cells     = (LineArray *)realloc(grid->cells, grid->width * grid->height * sizeof (LineArray)); 
}

static void
clearGrid(LineGrid *grid) {
    int size = grid->width * grid->height;

    for (int i = 0; i < size; ++i)
        grid->cells[i].clear();
}

static void
addLineToGrid(LineGrid *grid, Line line) {
    v2 a    = line.a / float(grid->cell_size);
    v2 b    = line.b / float(grid->cell_size);
    v2 iter = a;
    v2 dir  = 0.5f * norm(b - a);

    while (distSq(iter, b) > 1.0f) {
        LineArray *cell = grid->getCell(iter.x, iter.y);

        if (!cell->isFull()) {
            cell->add(line);
        }

        iter += dir;
    }
}

