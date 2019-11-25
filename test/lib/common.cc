#include "opencv2/opencv.hpp"

#include <cstdint>

#include <iostream>

#include <vector>
#include <cmath>
#include <algorithm>

#define RANGE(stl_container)    std::begin(stl_container), std::end(stl_container)

#define CLAMP_MIN(val, lo)      ((val) < (lo)? (lo) : (val))
#define CLAMP_MAX(val, hi)      ((val) > (hi)? (hi) : (val))

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
};

static int TilemapGet(const Tilemap *map, int x, int y)
{
    return map->tiles[y * map->width + x];
}

static void TilemapSet(Tilemap *map, int x, int y, int tile)
{
    map->tiles[y * map->width + x] = tile;
}

static void TilemapClear(Tilemap *map)
{
    memset(map->tiles, 0, (map->width * map->height) * sizeof (int));
}

static bool TilemapContains(const Tilemap *map, int x, int y)
{
    if (x < 0 || x >= map->width)  return false;
    if (y < 0 || y >= map->height) return false;

    return true;
}

static void TilemapResize(Tilemap *map, int image_width, int image_height, int cell_size)
{
    map->cell_size  = cell_size;
    map->width      = image_width  / map->cell_size;
    map->height     = image_height / map->cell_size;
    map->tiles      = (int *)realloc(map->tiles, map->width * map->height * sizeof (int));
}

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

static void TilemapDialate(Tilemap *map, int tile_type = TILE_EDGE)
{
    static int *old = NULL;

    old = (int *)realloc(old, map->width * map->height * sizeof (int));

    memcpy(old, map->tiles, map->width * map->height * sizeof (int));

    for (int ty = 0; ty < map->height; ++ty) {
        for (int tx = 0; tx < map->width; ++tx) {
            int tile = TilemapGet(map, tx, ty);

            if (tile != tile_type) {
                int sx = CLAMP_MIN(tx - 1, 0);
                int sy = CLAMP_MIN(ty - 1, 0);
                int ex = CLAMP_MAX(tx + 1, map->width - 1);
                int ey = CLAMP_MAX(ty + 1, map->height - 1);

                for (int y = sy; y <= ey; ++y) {
                    for (int x = sx; x <= ex; ++x) {
                        if (old[y * map->width + x] == tile_type) {
                            TilemapSet(map, tx, ty, tile_type);
                        }
                    }
                }
            }
        }
    }
}

static void TilemapErode(Tilemap *map, int tresh = 1, int tile_type = TILE_EDGE)
{
    static int *old = NULL;

    old = (int *)realloc(old, map->width * map->height * sizeof (int));

    memcpy(old, map->tiles, map->width * map->height * sizeof (int));

    for (int ty = 0; ty < map->height; ++ty) {
        for (int tx = 0; tx < map->width; ++tx) {
            int tile = TilemapGet(map, tx, ty);

            if (tile == tile_type) {
                int sx = CLAMP_MIN(tx - 1, 0);
                int sy = CLAMP_MIN(ty - 1, 0);
                int ex = CLAMP_MAX(tx + 1, map->width - 1);
                int ey = CLAMP_MAX(ty + 1, map->height - 1);

                int adj_count = 0;

                for (int y = sy; y <= ey; ++y) {
                    for (int x = sx; x <= ex; ++x) {
                        if (x == tx && y == ty) continue;

                        if (old[y * map->width + x] == tile_type) {
                            adj_count++;
                        }
                    }
                }

                if (adj_count < tresh) {
                    TilemapSet(map, tx, ty, TILE_NONE);
                }
            }
        }
    }
}

typedef int RoadState;

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
            if (TilemapGet(map, x, y) == TILE_ROAD) return y;
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

    for (int x = sx; x < ex; ++x) if (TilemapGet(map, x, sy) == TILE_ROAD) {
        result |= ROAD_UP;
    }

    for (int y = sy; y < ey; ++y) if (TilemapGet(map, sx, y) == TILE_ROAD) {
        result |= ROAD_LEFT;
    }

    for (int y = sy; y < ey; ++y) if (TilemapGet(map, ex, y) == TILE_ROAD) {
        result |= ROAD_RIGHT;
    }

    return result;
}

static float GetRoadPosition(const Tilemap *map)
{
    float center = 0.5f * map->width;

    int road_left  = 0;
    int road_right = map->width - 1;

    while (road_left < map->height && TilemapGet(map, road_left, map->height - 1) != TILE_ROAD) {
        road_left++;
    }

    while (road_right >= 0 && TilemapGet(map, road_right, map->height - 1) != TILE_ROAD) {
        road_right--;
    }

    float road_center = 0.5f * (road_right + road_left);

    return (center - road_center) / (0.5f * (road_right - road_left));
}

// ============================================ KLASSIFICATION ============================================== //

/*Pos
	* -1 -> 1//left -> right
*/

/* Type                  nr
	*	no way		     0
	*	single file way	 1
	*	up-left          3
	*	up-right         5
	*	left-right       6
	*   up-left-right    7
	*   two-files        9
	*/

struct IntersecPlacement {
	int type;
	float pos;
};

struct KlassList {
	static const int maxSize = 10;
	static const int typeTypes = 10;

	IntersecPlacement list[maxSize];

	int index = 0;
	int size = 0;

	float pos = 0;
	float posSum = 0;

	int type = 0;
	int typeCheck[typeTypes];


	float posPrev = 0;
	float posDifSum = 0;
	float posDif = 0;

	void push(IntersecPlacement newInput) {
		//at size 10
		if (size == maxSize) {

			posSum = posSum - list[index].pos;

			if (typeCheck[list[index].type] > 0) {
				typeCheck[list[index].type]--;
			}

			list[index] = newInput;
			posSum += list[index].pos;

			typeCheck[list[index].type]++;
			if (typeCheck[list[index].type] == (int)maxSize*0.8) {
				type = typeCheck[list[index].type];
			}
			pos = posSum / maxSize;

			index = (++index) % maxSize;
		}

		// at size 9
		else if (size == (maxSize-1)) {
			list[index] = newInput;
			posSum += newInput.pos;
			typeCheck[newInput.type]++;

			for (IntersecPlacement temp : list) {
				if (typeCheck[temp.type] > (int)maxSize*0.8) {
					type = temp.type;
				}
			}
			pos = posSum / maxSize;

			posDifSum += posPrev - newInput.pos;
			posPrev = newInput.pos;
			posDif = posDifSum / maxSize;

			index = (++index) % maxSize;
			size++;
		}
		//size below 9
		else {
			list[index] = newInput;
			posSum = posSum + newInput.pos;
			typeCheck[newInput.type]++;


			posDifSum += posPrev - newInput.pos;
			posPrev = newInput.pos;


			index++;
			size++;
		}
	}

    //1 = blink right, -1 = blink left
	int analyze() {
		switch (type) {
            case 0:
			    return 0;
		    case 1:
			    return 1;
		}

        return 0;
	}
};
