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

typedef unsigned char Pixel[3];

static float RSqrt(float n)
{
    return n == 0? 0 : sqrtf(n);
}

struct v2
{
    float   x;
    float   y;
};

static float Dot(v2 a, v2 b)
{
    return a.x * b.x + a.y * b.y;
}

static float LenSq(v2 a)
{
    return Dot(a, a);
}

static float Len(v2 a)
{
    return sqrtf(LenSq(a));
}

static v2 Norm(v2 a)
{
    float rlen = RSqrt(LenSq(a));

    a.x /= rlen;
    a.y /= rlen;

    return a;
}

// =================================================== KERNELS ====================================================== //

static const float kernel5_gaussian_blur[5][5] = {
    2.0f / 159.0f, 4.0f  / 159.0f, 5.0f  / 159.0f, 4.0f  / 159.0f, 2.0f / 159.0f,
    4.0f / 159.0f, 9.0f  / 159.0f, 12.0f / 159.0f, 9.0f  / 159.0f, 4.0f / 159.0f,
    5.0f / 159.0f, 12.0f / 159.0f, 15.0f / 159.0f, 12.0f / 159.0f, 5.0f / 159.0f,
    4.0f / 159.0f, 9.0f  / 159.0f, 12.0f / 159.0f, 9.0f  / 159.0f, 4.0f / 159.0f,
    2.0f / 159.0f, 4.0f  / 159.0f, 5.0f  / 159.0f, 4.0f  / 159.0f, 2.0f / 159.0f
};

static const float kernel5_box_blur[5][5] = {
    1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f,
    1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f,
    1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f,
    1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f,
    1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f
};

static const float kernel5_canny[5][5] = {
     -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1,
     -1, -1, 24, -1, -1,
     -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1
};

static const float kernel3_canny[3][3] = {
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
};

// ===================================== IMAGE PROCESSING ON C-TYPES ================================================= //
// NOTE(anton): Warning! these function assumes that dst has enough memory!

static void GrayscaleFromRgb(unsigned char *dst, const Pixel *src, int width, int height)
{
    int size = width * height;

    for (int i = 0; i < size; ++i) {
        const unsigned char *pixel = src[i];

        dst[i] = 0.3f * pixel[0] + 0.59f * pixel[1] + 0.11f * pixel[2];
    }
}

static void GrayscaleApplyKernel(unsigned char *dst, const unsigned char *src, int width, int height, const float kernel[3][3])
{
    for (int iy = 1; iy < height - 1; iy++) {
        for (int ix = 1; ix < width - 1; ix++) {
            int result = 0;

            for (int ky = 0; ky < 3; ky++) {
                int y = iy + ky - 1;

                for (int kx = 0; kx < 3; kx++) {
                    int x = ix + kx - 1;
                    result += kernel[ky][kx] * src[y * width + x];
                }
            }

            if (result < 0)   result = 0;
            if (result > 255) result = 255;
            
            dst[iy * width + ix] = result;
        }
    }
}

static void GrayscaleApplyKernel(unsigned char *dst, const unsigned char *src, int width, int height, const float kernel[5][5])
{
    for (int iy = 2; iy < height - 2; ++iy) {
        for (int ix = 2; ix < width - 2; ++ix) {
            int result = 0;
            //
            for (int ky = 0; ky < 5; ++ky) {
                int y = iy + ky - 2;

                for (int kx = 0; kx < 5; ++kx) {
                    int x = ix + kx - 2;
                    result += kernel[ky][kx] * src[y * width + x];
                }
            }

            if (result < 0)   result = 0;
            if (result > 255) result = 255;

            dst[iy * width + ix] = result;
        }
    }
}

// ==================================================================================================================== //

struct Rgb
{
    int     width;
    int     height;
    Pixel   *pixels;
};

struct Grayscale
{
    int             width;
    int             height;
    unsigned char   *pixels;
};

static Grayscale GrayscaleCreate(int width, int height)
{
    Grayscale gray;

    gray.width = width;
    gray.height = height;

    return gray;
}

static void GrayscaleResize(Grayscale *gray, int width, int height)
{
    gray->width = width;
    gray->height = height;
    gray->pixels = (unsigned char *)realloc(gray->pixels, gray->width * gray->height * sizeof (unsigned char));
}

static void GrayscaleFromRgb(Grayscale *gray, const Rgb *rgb)
{
    gray->width     = rgb->width;
    gray->height    = rgb->height;
    gray->pixels    = (unsigned char *)realloc(gray->pixels, gray->width * gray->height * sizeof (unsigned char));

    GrayscaleFromRgb(gray->pixels, rgb->pixels, gray->width, gray->height);
}

static void GrayscaleApplyKernel(Grayscale* dst, const Grayscale *src, const float kernel[3][3])
{
    int image_width  = src->width;
    int image_height = src->height;

    dst->width  = image_width;
    dst->height = image_height;
    dst->pixels = (unsigned char*)realloc(dst->pixels, image_width * image_height * sizeof (unsigned char));

    GrayscaleApplyKernel(dst->pixels, src->pixels, image_width, image_height, kernel);
}

static void GrayscaleApplyKernel(Grayscale* dst, const Grayscale *src, const float kernel[5][5])
{
    int image_width  = src->width;
    int image_height = src->height;

    dst->width  = image_width;
    dst->height = image_height;
    dst->pixels = (unsigned char*)realloc(dst->pixels, image_width * image_height * sizeof (unsigned char));

    GrayscaleApplyKernel(dst->pixels, src->pixels, image_width, image_height, kernel);
}

// ============================================ LINE GRID ============================================== //

typedef int TileType;

enum
{
    TILE_NONE,
    TILE_EDGE,
    TILE_ROAD
};

struct Tilemap
{
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

static void TilemapFillEdges(Tilemap *map, const unsigned char *data, int width, int height, int marker = TILE_EDGE)
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

static void TilemapFloodFill(Tilemap *map, int start_x, int start_y, int marker = TILE_ROAD)
{
    struct Point { int x, y; };

    static Point *point_stack   = NULL;

    point_stack = (Point *)realloc(point_stack, map->width * map->height * sizeof (Point));

    int point_count = 0;

    point_stack[point_count++] = { start_x, start_y };

    int start_tile = map->tiles[start_y * map->width + start_x];

    while (point_count) {
        Point current = point_stack[--point_count];

        map->tiles[current.y * map->width + current.x] = marker;

        const Point ns[4] = {
            current.x,     current.y - 1,
            current.x,     current.y + 1,
            current.x - 1, current.y,
            current.x + 1, current.y
        };

        for (int i = 0; i < ARRAY_COUNT(ns); ++i) {
            Point n = ns[i];

            if (n.x < 0 || n.x >= map->width)  continue;
            if (n.y < 0 || n.y >= map->height) continue;
            if (map->tiles[n.y * map->width + n.x] == marker)     continue;
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
enum
{
    ROAD_NONE       = (0),
    ROAD_UP         = (1 << 0),
    ROAD_LEFT       = (1 << 1),
    ROAD_RIGHT      = (1 << 2),
    ROAD_TWO_LANES  = (1 << 3),
};

static int GetRoadHeight(const Tilemap *map)
{
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            if (TilemapGet(map, x, y) == TILE_ROAD)
                return y;
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

    for (int x = sx; x < ex; ++x)
        if (TilemapGet(map, x, sy) == TILE_ROAD)
            result |= ROAD_UP;

    for (int y = sy; y < ey; ++y)
        if (TilemapGet(map, sx, y) == TILE_ROAD)
            result |= ROAD_LEFT;

    for (int y = sy; y < ey; ++y)
        if (TilemapGet(map, ex, y) == TILE_ROAD)
            result |= ROAD_RIGHT;

    if ((result & ROAD_UP) == result) {
        //
    }

    return result;
}

static float GetRoadPosition(const Tilemap *map)
{
    float center     = 0.5f * map->width;
    int   road_left  = 0;
    int   road_right = map->width - 1;

    while (road_left < map->height && TilemapGet(map, road_left, map->height - 1) != TILE_ROAD) {
        road_left++;
    }

    while (road_right >= 0 && TilemapGet(map, road_right, map->height - 1) != TILE_ROAD) {
        road_right--;
    }

    float road_center = 0.5f * (road_right + road_left);

    return (center - road_center) / (0.5f * (road_right - road_left));
}

static v2 GetRoadCenterVector(const Tilemap *map)
{
    int height = GetRoadHeight(map);

    v2 top_left  = { 0.0f,              0.0f };
    v2 top_right = { (float)map->width, 0.0f };
    v2 bot_left  = { 0.0f,              (float)map->height };
    v2 bot_right = { (float)map->width, (float)map->height };

    v2 center_top = { top_right.x - top_left.x, top_right.y - top_left.y };
    v2 center_bot = { bot_right.x - bot_left.x, bot_right.y - bot_left.y };

    v2 dir = { 0.0f, 0.0f };

    return Norm(dir);
}

// ============================================ KLASSIFICATION ============================================== //

/*Pos
	* -1 -> 1//left -> right
*/
/*      Type             #
	*	no way		     0
	*	single file way	 1
	*	up-left          3
	*	up-right         5
	*	left-right       6
	*   up-left-right    7
	*   two-files        8
	*/

struct InterPos {
	int type;
	float pos;
};

struct InterPosList {
	static const int maxSize = 10;
	static const int typeTypes = 10;
	static const int difListSize = (int)maxSize * 0.5;
	static const int typeThreshold = (int)maxSize * 0.8;

	InterPos list[maxSize];
	int index = 0;
	int size = 0;

	//position
	float pos = 0;
	float posSum = 0;

	//type of intersection/road scenario
	int type = 0;
	int typeCheck[typeTypes];
	
	//Difference in pos
	float posPrev = 0;
	float difList[difListSize];
	int difListIndex = 0;
	float posDifAvg = 0;

	//-1 left blink//1 right blink//0 no blink
	int blink = 0;

	void push(InterPos newInput) {
		//at size 10
		if (size == maxSize) {
			posSum = posSum - list[index].pos;

			if (typeCheck[list[index].type] > 0) {
				typeCheck[list[index].type]--;
			}

			list[index] = newInput;
			posSum += list[index].pos;

			typeCheck[list[index].type]++;
			if (typeCheck[list[index].type] == typeThreshold) {
				type = typeCheck[list[index].type];
			}
			pos = posSum / maxSize;


			//-----pos difference-----///
			difList[difListIndex] = newInput.pos - posPrev;
			float posDifSum = 0;
			
			for (float dif : difList) {
				posDifSum += dif;
			}
			posDifAvg = posDifSum / difListSize;

			posPrev = list[index].pos;
			difListIndex = (++difListIndex) % difListSize;
			///-----------------------////

			index = (++index) % maxSize;
		}

		// at size 9
		else if (size == (maxSize-1)) {

			list[index] = newInput;
			posSum += newInput.pos;
			typeCheck[newInput.type]++;

			for (InterPos temp : list) {
				if (typeCheck[temp.type] >= typeThreshold) {
					type = temp.type;
				}
			}
			pos = posSum / maxSize;


			difList[difListIndex] = newInput.pos - posPrev;
			float posDifSum = 0;
			for (float dif : difList) {
				posDifSum += dif;
			}
			posDifAvg = posDifSum / difListSize;

			posPrev = list[index].pos;

			index = (++index) % maxSize;
			difListIndex = (++difListIndex) % difListSize;
			size++;
		}
		//size below 9
		else {
			list[index] = newInput;
			posSum = posSum + newInput.pos;
			typeCheck[newInput.type]++;

			if (size != 0) {
				difList[difListIndex] = newInput.pos - posPrev;
				difListIndex = (++difListIndex) % difListSize;
			}
			if (size >= difListSize) {
				float posDifSum = 0;
				for (float dif : difList) {
					posDifSum += dif;
				}
				posDifAvg = posDifSum / difListSize;
			}
			
			posPrev = list[index].pos;
			index++;
			size++;
		}
	}



	void analyze() {
		switch (type) {
		case 0:
			blink = 0;
		case 1:
			blink = 0;
		case 2:
			
		case 3:
			leftUp();
		case 4:

		case 5:
			rightUp();
		case 6:
			leftRight();
		case 7:
			leftRightUp();
		}

	}

	/*
		Threeway intersecion
	*/
	void leftRight() {
		if (pos > 0) blink = 1;
		else blink = -1;
	}

	void leftUp() {
		if (pos < 0) blink = -1;
	}

	void rightUp() {
		if (pos < 0) blink = 1;
	}


	//4-wayintersection
	void leftRightUp() {
		if (pos > 0.5) blink = 1;
		else if (pos < -0.5)  blink = -1;
		else blink = 0;
	}

	//File change
	void twoFiles() {
		if (posDifAvg > -0.03) blink = -1;
		else if (posDifAvg > 0.03) blink = 1;
		else blink = 0;
	}
};
