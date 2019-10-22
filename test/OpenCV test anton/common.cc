#include "opencv2/opencv.hpp"

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <array>
#include <cmath>

// =============================================== MATH ================================================= //

struct Vector2 {
    float   x;
    float   y;
};

inline Vector2
operator+(const Vector2 &a, const Vector2 &b) {
    Vector2 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

inline Vector2
operator-(const Vector2 &a, const Vector2 &b) {
    Vector2 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

inline Vector2
operator*(const Vector2 &a, float s) {
    Vector2 r;
    r.x = a.x * s;
    r.y = a.y * s;
    return r;
}

inline Vector2
operator/(const Vector2 &a, float s) {
    Vector2 r;
    r.x = a.x / s;
    r.y = a.y / s;
    return r;
}

inline float
dot(const Vector2 &a, const Vector2 &b) {
    return a.x * b.x + a.y * b.y;
}

inline float
lenSq(const Vector2 &a) {
    return dot(a, a);
}

inline float
len(const Vector2 &a) {
    return std::sqrt(dot(a, a));
}

inline Vector2
norm(const Vector2 &a) {
    return a / len(a);
}

struct Line {
    Vector2 a;
    Vector2 b;
};

// ============================================ LINE GRID ============================================== //

class Grid {
    struct Cell {
        int count = 0;
        Line array[64];
    };

    int width = 0;
    int height = 0;
    Cell* cells = nullptr;

public:
    Grid(int width, int height);
    ~Grid();

    void clear();
    void addLine(const Line &line);
};

inline
Grid::Grid(int width, int height) {
    this->width  = width;
    this->height = height;
    this->cells  = new Cell[this->width * this->height];
}

inline
Grid::~Grid() {
    delete[] this->cells;
}

inline void
Grid::clear() {
    int size = this->width * this->height;

    for (int i = 0; i < size; ++i)
        cells[i].count = 0;
}

inline void
Grid::addLine(const Line &line) {
    auto delta = norm(line.b - line.a);
    auto iter  = line.a;

    while (int(iter.x) != int(line.b.x) && int(iter.y) != int(line.b.x)) {
        auto *cell = &cells[int(iter.y) * width + int(iter.x)];
        cell->array[cell->count++] = line;
    }
}

