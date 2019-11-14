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

enum TileType : int {
    TILE_NONE,
    TILE_LINE,
    TILE_ROAD
};

struct Tilemap {
    int     width;
    int     height;
    int     cell_size;
    //
    std::vector<int>   tiles;

    Tilemap(int cell_size) { this->cell_size = cell_size; }
    
    int get(int x, int y) const { return tiles[y * this->width + x]; }

    void clear() { std::fill(RANGE(tiles), 0); }

    bool contains(int x, int y) const {
        if (x < 0 || x >= this->width)  return false;
        if (y < 0 || y >= this->height) return false;
        return true;
    }

    void resize(int image_width, int image_height) {
        width  = image_width  / this->cell_size;
        height = image_height / this->cell_size;

        tiles.resize(this->width * this->height);
    }

    void addLine(cv::Vec4i cv_line, int line_marker = 1) {
        Line line = {
            float(cv_line[0]), float(cv_line[1]),
            float(cv_line[2]), float(cv_line[3])
        };

        auto cs   = float(this->cell_size);
        auto a    = line[0] / cs;
        auto b    = line[1] / cs;
        auto iter = a;
        auto dir  = 0.2f * norm(b - a);

        while (contains(iter[0], iter[1]) &&  distSq(iter, b) > 0.2f) {
            auto cell = this->get(iter[0], iter[1]);

            this->tiles[int(iter[1]) * this->width + int(iter[0])] = line_marker;

            iter[0] += dir[0];
            iter[1] += dir[1];
        }
    }
};

static void floodFill(Tilemap *map, int start_x, int start_y, int marker) {
    struct Point { int x, y; };

    std::vector<Point>  queue;
    std::vector<bool>   visited(map->width * map->height);

    queue.reserve(map->width * map->height);
    queue.push_back({ start_x, start_y });

    int start_tile = map->tiles[start_y * map->width + start_x];

    while (!queue.empty()) {
        Point current = queue[queue.size() - 1];
        queue.pop_back();

        map->tiles[current.y * map->width + current.x] = marker;
        visited[current.y * map->width + current.x]    = true;

        const std::array<Point, 4> ns = {
            current.x,     current.y - 1,
            current.x,     current.y + 1,
            current.x - 1, current.y,
            current.x + 1, current.y
        };

        for (auto n : ns) {
            if (n.x < 0 || n.x >= map->width)     continue;
            if (n.y < 0 || n.y >= map->height)    continue;
            if (visited[n.y * map->width + n.x])  continue;
            if (start_tile != map->tiles[n.y * map->width + n.x]) continue;

            queue.push_back(n);
        }
    }
}

using RoadState = int;

enum {
    ROAD_NONE       = (0),
    ROAD_UP         = (1 << 0),
    ROAD_LEFT       = (1 << 1),
    ROAD_RIGHT      = (1 << 2)
};

static int getRoadHeight(const Tilemap *map) {
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            if (map->get(x, y) == TILE_ROAD) return y;
        }
    }
    return 0;
}

static RoadState getRoadState(const Tilemap *map) {
    RoadState result = 0;

    int sx = 1;
    int sy = getRoadHeight(map) + 1;
    int ex = map->width  - 2;
    int ey = map->height - 2;

    for (int x = sx; x < ex; ++x) if (map->get(x, sy)) result |= ROAD_UP;
    for (int y = sy; y < ey; ++y) if (map->get(sx, y)) result |= ROAD_LEFT;
    for (int y = sy; y < ey; ++y) if (map->get(ex, y)) result |= ROAD_RIGHT;

    return result;
}

