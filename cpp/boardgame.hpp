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

inline bool onboard(int x, int y, int L)
{
    return x >= 0 && x < L && y >= 0 && y < L;
}

inline bool onboard(int pos, int L)
{
    return pos >= 0 && pos < L * L;
}

const int D2[4][2] = {
    {-1, 0},
    {0, -1},
    {0, 1},
    {1, 0},
};