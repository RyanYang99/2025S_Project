#pragma once

#define PERLIN_SIZE 256

extern int pPermutation_table[PERLIN_SIZE];

void perlin_fill_table(const int seed);
const float perlin_lerp(const float a, const float b, const float t);
const float perlin_noise(const float x);