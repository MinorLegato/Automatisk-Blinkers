#include "opencv2/opencv.hpp"

#include <cstdint>

#include <iostream>

#include <vector>
#include <cmath>
#include <algorithm>

#define RANGE(stl_container)    std::begin(stl_container), std::end(stl_container)

#define CLAMP(val, lo, hi)      ((val) < (lo)? (lo) : ((val > (hi)? (hi) : (val))))

#define ARRAY_COUNT(array)      (sizeof (array) / sizeof (array[0]))

// ========================================== UTIL FUNCS =============================================== //

static float RSqrt(float val)
{
    return val == 0.0f? 0.0f : 1.0f / std::sqrt(val);
}

static inline float Dot(const float a[2], const float b[2])
{
    return a[0] * b[0] + a[1] * b[1];
}

static inline float LenSq(const float a[2])
{
    return Dot(a, a);
}

static inline float Len(const float a[2])
{
    return std::sqrt(Dot(a, a));
}

static inline float DistSq(const float a[2], const float b[2])
{
    float c[2] = { b[0] - a[0], b[1] - a[1] };
    return c[0] * c[0] + c[1] * c[1];
}

static inline void Norm(float out[2], const float a[2])
{
    float rlen = RSqrt(LenSq(a));

    out[0] = a[0] * rlen;
    out[1] = a[1] * rlen;
}

// ============================================ LINE GRID ============================================== //

typedef int TileType;

enum {
    TILE_NONE,
    TILE_EDGE,
    TILE_ROAD
};

struct Tilemap {
    int     width;
    int     height;
    int     cell_size;
    int     *tiles;

    Tilemap(int cell_size)
    {
        this->cell_size = cell_size;
    }
    
    int Get(int x, int y) const
    {
        return tiles[y * width + x];
    }

    void Clear()
    {
        memset(tiles, 0, (width * height) * sizeof (int));
    }

    bool Contains(int x, int y) const
    {
        if (x < 0 || x >= width)  return false;
        if (y < 0 || y >= height) return false;
        return true;
    }

    void Resize(int image_width, int image_height)
    {
        width  = image_width  / cell_size;
        height = image_height / cell_size;

        tiles  = (int *)realloc(tiles, width * height * sizeof (int));
    }

    void AddLine(cv::Vec4i cv_line, int line_marker = TILE_EDGE)
    {
        float cs      = cell_size;
        float a[2]    = { cv_line[0] / cs, cv_line[1] / cs };
        float b[2]    = { cv_line[2] / cs, cv_line[3] / cs };
        float iter[2] = { a[0], a[1] };
        float dir[2]  = { b[0] - a[0], b[1] - a[1] };

        Norm(dir, dir);

        while (Contains(iter[0], iter[1]) &&  DistSq(iter, b) > 0.2f) {
            auto cell = Get(iter[0], iter[1]);

            tiles[int(iter[1]) * this->width + int(iter[0])] = line_marker;

            iter[0] += 0.2f * dir[0];
            iter[1] += 0.2f * dir[1];
        }
    }
};

static void TilemapFill(Tilemap *map, const unsigned char *data, int width, int height, int marker = TILE_EDGE)
{
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

static void FloodFill(Tilemap *map, int start_x, int start_y, int marker = TILE_ROAD)
{
    struct Point { int x, y; };

    static Point *point_stack   = NULL;
    static int   *visited       = NULL;

    point_stack    = (Point *)realloc(point_stack, map->width * map->height * sizeof (Point));
    visited        = (int *)realloc(visited, map->width * map->height * sizeof (int));

    int point_count = 0;
    memset(visited, 0, map->width * map->height * sizeof (int));

    point_stack[point_count++] = { start_x, start_y };

    int start_tile = map->tiles[start_y * map->width + start_x];

    while (point_count) {
        Point current = point_stack[--point_count];

        map->tiles[current.y * map->width + current.x] = marker;
        visited[current.y * map->width + current.x]    = true;

        const Point ns[4] = {
            current.x,     current.y - 1,
            current.x,     current.y + 1,
            current.x - 1, current.y,
            current.x + 1, current.y
        };

        for (int i = 0; i < ARRAY_COUNT(ns); ++i) {
            Point n = ns[i];

            if (n.x < 0 || n.x >= map->width)     continue;
            if (n.y < 0 || n.y >= map->height)    continue;
            if (visited[n.y * map->width + n.x])  continue;
            if (start_tile != map->tiles[n.y * map->width + n.x]) continue;

            point_stack[point_count++] = n;
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

static int GetRoadHeight(const Tilemap *map)
{
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            if (map->Get(x, y) == TILE_ROAD) return y;
        }
    }
    return 0;
}

static RoadState GetRoadState(const Tilemap *map)
{
    RoadState result = 0;

    int sx = 1;
    int sy = GetRoadHeight(map) + 1;
    int ex = map->width  - 2;
    int ey = map->height - 2;

    for (int x = sx; x < ex; ++x) if (map->Get(x, sy) == TILE_ROAD) {
        result |= ROAD_UP;
    }

    for (int y = sy; y < ey; ++y) if (map->Get(sx, y) == TILE_ROAD) {
        result |= ROAD_LEFT;
    }

    for (int y = sy; y < ey; ++y) if (map->Get(ex, y) == TILE_ROAD) {
        result |= ROAD_RIGHT;
    }

    return result;
}

static float GetRoadPosition(const Tilemap *map)
{
    float center = 0.5f * map->width;

    int road_left  = 0;
    int road_right = map->width - 1;

    while (road_left < map->height && map->Get(road_left, map->height - 1) != TILE_ROAD) {
        road_left++;
    }

    while (road_right >= 0 && map->Get(road_right, map->height - 1) != TILE_ROAD) {
        road_right--;
    }

    float road_center = 0.5f * (road_right + road_left);

    return (center - road_center) / (0.5f * (road_right - road_left));
}

// ============================================ KLASSIFICATION ============================================== //


struct IntersecPlacement {
	int     type;
	float   pos;
};

// 0 = no blink// 1 = right blink // -1 = left blink
int Klass(std::vector<IntersecPlacement> &que)
{
	int   type_sum = 0;
	float pos_sum  = 0;
	float pos_avg  = 0;
	float type_avg = 0;

	for(IntersecPlacement temp : que) {
		pos_sum  = pos_sum + temp.pos;
		type_sum = type_sum + temp.type;
	}

	pos_avg  = pos_sum;
	type_avg = type_sum / que.size();

	if (pos_avg > 0.7)  return 1;
	if (pos_avg < -0.7) return -1;

    return 0;
}
