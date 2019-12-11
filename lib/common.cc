#include "opencv2/opencv.hpp"

#include <cstdint>

#include <iostream>

#include <vector>
#include <cmath>
#include <algorithm>

#define CLAMP_MIN(val, lo)      ((val) < (lo)? (lo) : (val))
#define CLAMP_MAX(val, hi)      ((val) > (hi)? (hi) : (val))

#define CLAMP(val, lo, hi)      ((val) < (lo)? (lo) : ((val > (hi)? (hi) : (val))))

#define ARRAY_COUNT(array)      (sizeof (array) / sizeof (array[0]))

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

static float DistSq(v2 a, v2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    return dx * dx + dy * dy;
}

static float Dist(v2 a, v2 b)
{
    return sqrtf(DistSq(a, b));
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

typedef unsigned char Pixel[3];

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
    TILE_ROAD,
    TILE_ROAD_EDGE,
    TILE_CENTER,
    TILE_LANE_CENTER
};

struct Tilemap
{
    int32_t     width;
    int32_t     height;
    int32_t     cell_size;
    uint8_t     *tiles;
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
    memset(map->tiles, 0, (map->width * map->height) * sizeof *map->tiles);
}

static void TilemapResize(Tilemap *map, int image_width, int image_height, int cell_size)
{
    map->cell_size  = cell_size;
    map->width      = image_width  / map->cell_size;
    map->height     = image_height / map->cell_size;
    map->tiles      = (decltype(map->tiles))realloc(map->tiles, map->width * map->height * sizeof *map->tiles);
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

static void TilemapFloodFill(Tilemap* dst, const Tilemap *map, int start_x, int start_y, int marker = TILE_ROAD)
{
    struct Point { int x, y; };

    static Point *point_stack   = NULL;

    point_stack = (Point *)realloc(point_stack, map->width * map->height * sizeof (Point));

    int point_count = 0;

    point_stack[point_count++] = { start_x, start_y };

    int start_tile = map->tiles[start_y * map->width + start_x];

    while (point_count) {
        Point current = point_stack[--point_count];

        dst->tiles[current.y * map->width + current.x] = marker;

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
            if (dst->tiles[n.y * map->width + n.x] == marker)     continue;
            if (start_tile != map->tiles[n.y * map->width + n.x]) continue;

            point_stack[point_count++] = n;
        }
    }
}

static void TilemapFloodFillRoad(Tilemap* dst, const Tilemap *map, int start_x, int start_y)
{
    struct Point { int x, y; };

    static Point *point_stack   = NULL;

    point_stack = (Point *)realloc(point_stack, map->width * map->height * sizeof (Point));

    int point_count = 0;

    point_stack[point_count++] = { start_x, start_y };

    int start_tile = map->tiles[start_y * map->width + start_x];

    while (point_count) {
        Point current = point_stack[--point_count];

        dst->tiles[current.y * map->width + current.x] = TILE_ROAD;

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

            if (dst->tiles[n.y * map->width + n.x] == TILE_ROAD) continue;

            if (start_tile != map->tiles[n.y * map->width + n.x]) {
                dst->tiles[n.y * map->width + n.x] = TILE_ROAD_EDGE;
                continue;
            }

            point_stack[point_count++] = n;
        }
    }
}

static void TilemapDialate(Tilemap *map, int tile_type)
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

static void TilemapErode(Tilemap *map, int tresh, int tile_type)
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

static void TilemapDrawLine(Tilemap *map, v2 start, v2 end, int pen = TILE_EDGE)
{
    v2 dir  = Norm({ end.x - start.x, end.y - start.y });
    v2 iter = start;

    while (DistSq(iter, end) > 1.0f) {
        TilemapSet(map, iter.x, iter.y, TILE_EDGE);

        iter.x += 0.5f * dir.x;
        iter.y += 0.5f * dir.y;
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

static int TilemapGetRoadHeight(const Tilemap *map)
{
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            if (TilemapGet(map, x, y) == TILE_ROAD)
                return y;
        }
    }

    return 0;
}

static float TilemapGetRoadPosition(const Tilemap *map, RoadState state)
{
    float   center     = 0.5f * map->width;
    int     road_left  = 0;
    int     road_right = map->width - 1;

    while (road_left < map->height && TilemapGet(map, road_left, map->height - 1) != TILE_ROAD) {
        road_left++;
    }

    while (road_right >= 0 && TilemapGet(map, road_right, map->height - 1) != TILE_ROAD) {
        road_right--;
    }

    float road_center = 0.5f * (road_right + road_left);
    float road_position = (center - road_center) / (0.5f * (road_right - road_left));

    return road_position;

    // @TODO: handle two lanes by returning the center of the current lane!
#if 0
    if (state & ROAD_TWO_LANES) {
        //
    } else {
    }
#endif
}

// draw the center of the road into the 'dst' tilemap.
// returns a float between 0.0f - 1.0f, that reprecents the precentage of the center that was not part of the road.
static float TilemapDrawRoadCenter(Tilemap *dst, const Tilemap *map, int center_width = 0)
{
    int tiles_total = 0;
    int tiles_edge  = 0;

    int height = TilemapGetRoadHeight(map);

    for (int y = height; y < map->height; ++y) {
        int left  = 0.5f * map->width;
        int right = 0.5f * map->width;

        for (int x = 0; x < map->width; ++x) {
            if (TilemapGet(map, x, y) == TILE_ROAD) {
                if (x < left)  left  = x;
                if (x > right) right = x;
            }
        }

        int center       = 0.5f * (left  + right);
        int left_center  = 0.5f * (left  + center);
        int right_center = 0.5f * (right + center);

        TilemapSet(dst, left_center,  y, TILE_LANE_CENTER);
        TilemapSet(dst, right_center, y, TILE_LANE_CENTER);

        int start = center - center_width;
        int end   = center + center_width;

        for (int i = start; i <= end; ++i) {
            tiles_total++;

            if (TilemapGet(map, i, y) == TILE_EDGE)
                tiles_edge++;

            TilemapSet(dst, i, y, TILE_CENTER);
        }
    }

    return tiles_total? (float)tiles_edge / (float)tiles_total : 0.0f;
}

static bool TilemapIsRoadHorizontalAt(const Tilemap *map, int x)
{
    int prev_tile   = TILE_NONE;

    for (int y = 0; y < map->height; ++y) {
        int tile = TilemapGet(map, x, y);

        if (prev_tile == TILE_ROAD && tile != TILE_ROAD)
            return true;

        prev_tile = tile;
    }

    return false;
}

static bool TilemapIsRoadVerticalAt(const Tilemap *map, int y)
{
    bool edge_left  = false;
    bool edge_right = false;

    for (int x = 1; x < map->width - 1; ++x) {
        if (TilemapGet(map, x, y) == TILE_ROAD && TilemapGet(map, x - 1, y) != TILE_ROAD) { 
            edge_left  = true;
        }

        if (TilemapGet(map, x, y) == TILE_ROAD && TilemapGet(map, x + 1, y) != TILE_ROAD) {
            edge_right = true;
        }
    }

    return edge_left && edge_right;
}

static RoadState TilemapGetRoadState(const Tilemap *map)
{
    RoadState result = 0;

    int sx  = 1;
    int sy  = TilemapGetRoadHeight(map);
    int ex  = map->width  - 2;

    if (TilemapIsRoadHorizontalAt(map, sx)) result |= ROAD_LEFT;
    if (TilemapIsRoadHorizontalAt(map, ex)) result |= ROAD_RIGHT;
    if (TilemapIsRoadVerticalAt(map, sy))   result |= ROAD_UP;

    return result;
}

// ================================================= CRC32 ============================================== //

static const uint32_t crc_table[] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

static uint32_t CRC32Code(const void *data, size_t size)
{
    const uint8_t *d  = data;
    uint32_t      crc = 0xFFFFFFFF;

    while (size--)
    {
        uint32_t index = (crc ^ *(d++)) & 0xFF;

        crc = (crc >> 8) ^ crc_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}


