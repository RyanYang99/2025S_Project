#include "leak.hpp"
#include "perlin.hpp"

#include <cmath>
#include <cstdlib>

int Perlin::permutation_table_[perlin_size]{ 170, 22, 164, 57, 210, 206, 153, 48, 0, 85, 196, 216, 60, 165, 255, 157, 107, 21, 73, 72, 36, 140, 247, 66, 176, 71, 31, 79, 75, 6, 200, 253, 7, 180, 112, 9, 251, 131, 235, 209, 27, 241, 240, 87, 207, 23, 17, 70, 81, 249, 191, 194, 230, 88, 25, 76, 129, 203, 97, 108, 135, 173, 26, 219, 227, 37, 116, 58, 47, 133, 181, 74, 14, 8, 205, 103, 94, 62, 218, 113, 214, 65, 128, 144, 134, 120, 3, 80, 184, 121, 67, 160, 187, 19, 189, 30, 122, 237, 143, 51, 69, 49, 231, 174, 54, 211, 177, 18, 4, 179, 197, 20, 126, 150, 228, 59, 83, 186, 11, 234, 119, 145, 63, 84, 115, 220, 5, 193, 239, 96, 24, 159, 225, 33, 147, 105, 171, 202, 155, 183, 61, 238, 198, 142, 213, 100, 35, 93, 204, 124, 222, 223, 162, 101, 39, 92, 141, 68, 190, 64, 169, 41, 244, 117, 90, 137, 167, 236, 243, 172, 32, 217, 86, 151, 99, 229, 226, 34, 44, 182, 10, 1, 52, 102, 139, 16, 110, 245, 118, 208, 252, 201, 114, 233, 242, 248, 175, 250, 215, 156, 12, 246, 132, 254, 192, 42, 91, 28, 95, 106, 152, 125, 199, 98, 89, 50, 163, 43, 148, 123, 149, 109, 45, 46, 136, 161, 195, 78, 111, 127, 2, 77, 154, 168, 221, 146, 232, 224, 178, 188, 130, 138, 104, 166, 40, 55, 158, 56, 82, 13, 185, 29, 38, 212, 15, 53 };

float Perlin::grad(const int p) noexcept {
    return (static_cast<float>(permutation_table_[p % perlin_size]) / static_cast<float>(perlin_size)) > 0.5f ? 1.0f : -1.0f;
}

void Perlin::permutation_table(const int _permutation_table[perlin_size]) noexcept {
    for (size_t i = 0; i < perlin_size; ++i)
        permutation_table_[i] = _permutation_table[i];
}

int Perlin::at(const size_t index) noexcept {
    return permutation_table_[index];
}

void Perlin::fill(const int seed) noexcept {
    srand(seed);

    for (int i{}; i < static_cast<int>(perlin_size); ++i)
        permutation_table_[i] = i;

    for (size_t i{}; i < perlin_size; ++i) {
        const int j{ rand() % perlin_size }, temp{ permutation_table_[i] };
        permutation_table_[i] = permutation_table_[j];
        permutation_table_[j] = temp;
    }
}

float Perlin::lerp(const float a, const float b, const float t) noexcept {
    return a + t * (b - a);
}

static constexpr float fade(const float t) noexcept {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Perlin::noise(const float x) noexcept {
    const int xi{ static_cast<int>(floorf(x)) };
    const float xf{ x - xi };

    return lerp(grad(xi) * xf, grad(xi + 1) * (xf - 1.0f), fade(xf));
}