#include "opencv2/opencv.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdint>

#include <vector>
#include <cmath>

using f32 = float;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// ========================================== MATH FUNCS =============================================== //

static inline f32 distSq2f(const f32 a[2], const f32 b[2]) {
    const f32 c[2] = { b[0] - a[0], b[1] - a[1] };
    return c[0] * c[0] + c[1] * c[1];
}

// ============================================ LINE GRID ============================================== //

#define LINE_CELL_MAX_COUNT (64)

struct LineCell {
    int     count;
    f32     array[LINE_CELL_MAX_COUNT][4];
};

struct LineGrid {
    int         width;
    int         height;
    int         cell_size;
    LineCell    *cells;
    //
    LineCell *getCell(int x, int y) {
        return &cells[y * width + x];
    }

    const LineCell *getCell(int x, int y) const {
        return &cells[y * width + x];
    }
};

static void initGrid(LineGrid *grid, int cell_size) {
    grid->cell_size     = cell_size;
    grid->cells         = nullptr;
}

static void resizeGrid(LineGrid *grid, int image_width, int image_height) {
    grid->width     = image_width  / grid->cell_size;
    grid->height    = image_height / grid->cell_size;
    grid->cells     = (LineCell *)realloc(grid->cells, grid->width * grid->height * sizeof (LineCell)); 
}

static void clearGrid(LineGrid *grid) {
    int size = grid->width * grid->height;

    for (int i = 0; i < size; ++i)
        grid->cells[i].count = 0;
}

static void addLineToGrid(LineGrid *grid, cv::Vec4i cv_line) {
    f32 cell_size = float(grid->cell_size);
    f32 a[2]      = { cv_line[0] / cell_size, cv_line[1] / cell_size };
    f32 b[2]      = { cv_line[2] / cell_size, cv_line[3] / cell_size };
    f32 iter[2]   = { a[0], a[1] };
    f32 dir[2]    = { b[0] - a[0], b[1] - a[1] };
    f32 dlen      = sqrtf(dir[0] * dir[0] + dir[1] * dir[1]);
    f32 inv_dlen  = dlen == 0.0f? 0.0f : 1.0f / dlen;

    dir[0] *= 0.5f * inv_dlen;
    dir[1] *= 0.5f * inv_dlen;

    while (distSq2f(iter, b) > 1.0f) {
        LineCell *cell = grid->getCell(iter[0], iter[1]);

        if (cell->count <= LINE_CELL_MAX_COUNT) {
            f32 *line = cell->array[cell->count++];

            line[0] = cv_line[0];
            line[1] = cv_line[1];
            line[2] = cv_line[2];
            line[3] = cv_line[3];
        }

        iter[0] += dir[0];
        iter[1] += dir[1];
    }
}

