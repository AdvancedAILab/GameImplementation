#pragma once

// common code for two-player board game

#include <array>

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;
const int WALL = 3;

inline int opponent(int color)
{
    return BLACK + WHITE - color;
}

template <size_t N>
std::array<int, N> operator +(const std::array<int, N>& a, const std::array<int, N>& b)
{
    std::array<int, N> c;
    for (size_t i = 0; i < N; i++) c[i] = a[i] + b[i];
    return c;
}

inline bool onboard_xy(int x, int y, int L)
{
    return x >= 0 && x < L && y >= 0 && y < L;
}

inline bool onboard_xy(int x, int y, int LX, int LY)
{
    return x >= 0 && x < LX && y >= 0 && y < LY;
}

inline bool onboard(int pos, int L)
{
    return pos >= 0 && pos < L * L;
}

inline bool onboard(int pos, int LX, int LY)
{
    return pos >= 0 && pos < LX * LY;
}

// direction

// 4 0 5
// 1 - 2
// 6 3 7

const int D2[8][2] = {
    {-1,  0},
    { 0, -1},
    { 0,  1},
    { 1,  0},
    {-1, -1},
    {-1,  1},
    { 1, -1},
    { 1,  1},
};

inline int point_symmetry(int d)
{
    return (d / 4) * 4 + 3 - (d % 4);
}