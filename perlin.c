#include "leak.h"
#include "perlin.h"

#include <math.h>
#include <stdlib.h>

int pPermutation_table[PERLIN_SIZE] = { 170, 22, 164, 57, 210, 206, 153, 48, 0, 85, 196, 216, 60, 165, 255, 157, 107, 21, 73, 72, 36, 140, 247, 66, 176, 71, 31, 79, 75, 6, 200, 253, 7, 180, 112, 9, 251, 131, 235, 209, 27, 241, 240, 87, 207, 23, 17, 70, 81, 249, 191, 194, 230, 88, 25, 76, 129, 203, 97, 108, 135, 173, 26, 219, 227, 37, 116, 58, 47, 133, 181, 74, 14, 8, 205, 103, 94, 62, 218, 113, 214, 65, 128, 144, 134, 120, 3, 80, 184, 121, 67, 160, 187, 19, 189, 30, 122, 237, 143, 51, 69, 49, 231, 174, 54, 211, 177, 18, 4, 179, 197, 20, 126, 150, 228, 59, 83, 186, 11, 234, 119, 145, 63, 84, 115, 220, 5, 193, 239, 96, 24, 159, 225, 33, 147, 105, 171, 202, 155, 183, 61, 238, 198, 142, 213, 100, 35, 93, 204, 124, 222, 223, 162, 101, 39, 92, 141, 68, 190, 64, 169, 41, 244, 117, 90, 137, 167, 236, 243, 172, 32, 217, 86, 151, 99, 229, 226, 34, 44, 182, 10, 1, 52, 102, 139, 16, 110, 245, 118, 208, 252, 201, 114, 233, 242, 248, 175, 250, 215, 156, 12, 246, 132, 254, 192, 42, 91, 28, 95, 106, 152, 125, 199, 98, 89, 50, 163, 43, 148, 123, 149, 109, 45, 46, 136, 161, 195, 78, 111, 127, 2, 77, 154, 168, 221, 146, 232, 224, 178, 188, 130, 138, 104, 166, 40, 55, 158, 56, 82, 13, 185, 29, 38, 212, 15, 53 };

void perlin_fill_table(const int seed) {
    srand(seed);

    for (int i = 0; i < PERLIN_SIZE; ++i)
        pPermutation_table[i] = i;

    for (int i = 0; i < PERLIN_SIZE; ++i) {
        const int j = rand() % PERLIN_SIZE;
        int temp = pPermutation_table[i];
        pPermutation_table[i] = pPermutation_table[j];
        pPermutation_table[j] = temp;
    }
}

const float perlin_lerp(const float a, const float b, const float t) {
    return a + t * (b - a);
}

static const float fade(const float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static const float grad(const int p) {
    return ((float)pPermutation_table[p % PERLIN_SIZE] / (float)PERLIN_SIZE) > 0.5f ? 1.0f : -1.0f;
}

const float perlin_noise(const float x) {
    const int xi = (int)floorf(x);
    const float xf = x - xi;

    return perlin_lerp(grad(xi) * xf, grad(xi + 1) * (xf - 1.0f), fade(xf));
}