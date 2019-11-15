#include "opencv2/opencv.hpp"

#include <cstdint>

#include <iostream>

#include <vector>
#include <cmath>
#include <algorithm>

#define RANGE(stl_container)    std::begin(stl_container), std::end(stl_container)

#define CLAMP(val, lo, hi)      ((val) < (lo)? (lo) : ((val > (hi)? (hi) : (val))))

// ========================================== UTIL FUNCS =============================================== //

template <typename T>
static float rsqrt(T val) {
    return val == T(0)? T(0) : T(1) / std::sqrt(val);
}

static inline float dot(const float a[2], const float b[2]) {
    return a[0] * b[0] + a[1] * b[1];
}

static inline float lenSq(const float a[2]) {
    return dot(a, a);
}

static inline float len(const float a[2]) {
    return std::sqrt(dot(a, a));
}

static inline float distSq(const float a[2], const float b[2]) {
    float c[2] = { b[0] - a[0], b[1] - a[1] };
    return c[0] * c[0] + c[1] * c[1];
}

static inline void norm(float out[2], const float a[2]) {
    float rlen = rsqrt(lenSq(a));

    out[0] = a[0] * rlen;
    out[1] = a[1] * rlen;
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

    void clear() {
        std::fill(RANGE(tiles), 0);
    }

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

    void addLine(cv::Vec4i cv_line, int line_marker = TILE_LINE) {
        float cs      = this->cell_size;
        float a[2]    = { cv_line[0] / cs, cv_line[1] / cs };
        float b[2]    = { cv_line[2] / cs, cv_line[3] / cs };
        float iter[2] = { a[0], a[1] };
        float dir[2]  = { b[0] - a[0], b[1] - a[1] };

        norm(dir, dir);

        while (contains(iter[0], iter[1]) &&  distSq(iter, b) > 0.2f) {
            auto cell = this->get(iter[0], iter[1]);

            this->tiles[int(iter[1]) * this->width + int(iter[0])] = line_marker;

            iter[0] += 0.2f * dir[0];
            iter[1] += 0.2f * dir[1];
        }
    }
};

static void tilemapFill(Tilemap *map, const unsigned char *data, int width, int height, int marker = TILE_LINE) {
    float inv_cell_size = 1.0f / map->cell_size;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (data[y * width + x]) {
                int tx = CLAMP(x * inv_cell_size, 0, map->width - 1);
                int ty = CLAMP(y * inv_cell_size, 0, map->height - 1);

                map->tiles[ty * map->width + tx] = marker;
            }
        }
    }
}

static void floodFill(Tilemap *map, int start_x, int start_y, int marker = TILE_ROAD) {
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

        const Point ns[4] = {
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
    ROAD_NONE  = (0),
    ROAD_UP    = (1 << 0),
    ROAD_LEFT  = (1 << 1),
    ROAD_RIGHT = (1 << 2)
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

    for (int x = sx; x < ex; ++x) if (map->get(x, sy) == TILE_ROAD) result |= ROAD_UP;
    for (int y = sy; y < ey; ++y) if (map->get(sx, y) == TILE_ROAD) result |= ROAD_LEFT;
    for (int y = sy; y < ey; ++y) if (map->get(ex, y) == TILE_ROAD) result |= ROAD_RIGHT;

    return result;
}

static float getRoadPosition(const Tilemap *map) {
    float center = 0.5f * map->width;

    int road_left   = 0;
    int road_right  = map->width - 1;

    while (map->get(road_left,  map->height - 1) != TILE_ROAD) road_left++;
    while (map->get(road_right, map->height - 1) != TILE_ROAD) road_right--;

    float road_center = 0.5f * (road_right + road_left);

    return center - road_center;
}

