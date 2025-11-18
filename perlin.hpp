#pragma once

class Perlin {
public:
    static constexpr size_t perlin_size{ 256 };

private:
    static int permutation_table_[perlin_size];

    static float grad(const int p) noexcept;

public:
    static void permutation_table(const int _permutation_table[perlin_size]) noexcept;
    static int at(const size_t index) noexcept;

    static void fill(const int seed) noexcept;
    static float lerp(const float a, const float b, const float t) noexcept;

    static float noise(const float x) noexcept;
};