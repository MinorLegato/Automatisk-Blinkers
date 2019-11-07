#include "opencv2/opencv.hpp"

#include <cstdint>

#include <iostream>

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

#define RANGE(stl_container)    std::begin(stl_container), std::end(stl_container)

// ========================================== UTIL FUNCS =============================================== //

template <typename T>
static float rsqrt(T val) {
    return val == T(0)? T(0) : T(1) / std::sqrt(val);
}

using Vector2 = std::array<float, 2>;
using Line    = std::array<Vector2, 2>;

static Vector2 operator+(const Vector2& a, const Vector2& b) { return { a[0] + b[0], a[1] + b[1] }; }
static Vector2 operator-(const Vector2& a, const Vector2& b) { return { a[0] - b[0], a[1] - b[1] }; }
static Vector2 operator*(const Vector2& a, float s) { return { a[0] * s, a[1] * s }; }
static Vector2 operator*(float s, const Vector2& a) { return { a[0] * s, a[1] * s }; }
static Vector2 operator/(const Vector2& a, float s) { return { a[0] / s, a[1] / s }; }

static void operator+=(Vector2& a, const Vector2& b) { a = a - b; }
static void operator-=(Vector2& a, const Vector2& b) { a = a - b; }
static void operator*=(Vector2& a, const float s) { a = a * s; }
static void operator/=(Vector2& a, const float s) { a = a / s; }

static inline float dot(const Vector2& a, const Vector2& b) {
    return a[0] * b[0] + a[1] * b[1];
}

static inline float lenSq(const Vector2& a) {
    return dot(a, a);
}

static inline float len(const Vector2& a) {
    return std::sqrt(dot(a, a));
}

static inline float distSq(const Vector2& a, const Vector2& b) {
    Vector2 c = { b[0] - a[0], b[1] - a[1] };
    return c[0] * c[0] + c[1] * c[1];
}

static inline Vector2 norm(const Vector2 a) {
    return a * rsqrt(lenSq(a));
}

// ============================================ LINE GRID ============================================== //

#define LINE_CELL_MAX_COUNT (1)

struct LineCell {
    int32_t     count;
    Line        array[LINE_CELL_MAX_COUNT];
    //
    void clear() { count = 0; }
};

struct LineGrid {
    int32_t                 width;
    int32_t                 height;
    int32_t                 cell_size;
    std::vector<LineCell>   cells;

    LineGrid(int cell_size) {
        this->cell_size = cell_size;
    }

    LineCell *get(int x, int y) {
        return &cells[y * this->width + x];
    }

    const LineCell *get(int x, int y) const {
        return &cells[y * this->width + x];
    }

    bool contains(int x, int y) const {
        if (x < 0 || x >= this->width)  return false;
        if (y < 0 || y >= this->height) return false;
        return true;
    }

    void resize(int image_width, int image_height) {
        width  = image_width  / this->cell_size;
        height = image_height / this->cell_size;

        cells.resize(this->width * this->height);
    }

    void clear() {
        int size = this->width * this->height;
        std::for_each(RANGE(this->cells), [] (auto& cell) { cell.clear(); });
    }

    void addLine(cv::Vec4i cv_line) {
        float cs = (float)this->cell_size;

        Vector2 a       = { cv_line[0] / cs, cv_line[1] / cs };
        Vector2 b       = { cv_line[2] / cs, cv_line[3] / cs };
        Vector2 iter    = a;
        Vector2 dir     = 0.5f * norm(b - a);

        while (contains(iter[0], iter[1]) &&  distSq(iter, b) > 1.0f) {
            LineCell *cell = this->get(iter[0], iter[1]);

            if (cell->count < LINE_CELL_MAX_COUNT) {
                auto& line = cell->array[cell->count++];

                line[0][0] = cv_line[0];
                line[0][1] = cv_line[1];
                line[1][0] = cv_line[2];
                line[1][1] = cv_line[3];
            }

            iter[0] += dir[0];
            iter[1] += dir[1];
        }
    }
};


enum RoadState {
    ROAD_STATE_NONE,
    ROAD_STATE_ROAD,
    ROAD_STATE_LANE,
    ROAD_STATE_THREE_WAY,
    ROAD_STATE_THREE_WAY_LEFT,
    ROAD_STATE_THREE_WAY_RIGHT,
    ROAD_STATE_FOUR_WAY,
    ROAD_STATE_COUNT
};

static RoadState getRoadState(const LineGrid& grid) {
    int sx = 1;
    int sy = 1;
    int ex = grid.width  - 2;
    int ey = grid.height - 2;

    int line_count_top   = 0;
    int line_count_bot   = 0;
    int line_count_left  = 0;
    int line_count_right = 0;

    for (int x = sx; x < ex; ++x) line_count_top   += grid.get(x, sy)->count;
    for (int x = sx; x < ex; ++x) line_count_bot   += grid.get(x, ey)->count;
    for (int y = sy; y < ey; ++y) line_count_left  += grid.get(sx, y)->count;
    for (int y = sy; y < ey; ++y) line_count_right += grid.get(ex, y)->count;

    std::cout << "top:   " << line_count_top   << '\n';
    std::cout << "bot:   " << line_count_bot   << '\n';
    std::cout << "left:  " << line_count_left  << '\n';
    std::cout << "right: " << line_count_right << '\n';

    return ROAD_STATE_NONE;
}

