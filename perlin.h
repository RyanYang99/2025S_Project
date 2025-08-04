#pragma once

#define PERLIN_SIZE 256

extern int permutation_table[PERLIN_SIZE];

void fill_table(const int seed);
const float lerp(const float a, const float b, const float t);
const float perlin_noise(const float x);