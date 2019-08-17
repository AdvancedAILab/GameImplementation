#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>

static int find(const std::string& s, char sep, std::string::size_type p = 0)
{
    std::string::size_type q = s.find(sep, p);
    if (q == std::string::npos) return -1;
    return q;
}

static std::vector<std::string> split(const std::string& s, char sep)
{
    std::vector<std::string> result;
    std::string::size_type p = 0, q;
    while ((q = s.find(sep, p)) != std::string::npos) {
        result.emplace_back(s, p, q - p);
        p = q + 1;
    }
    result.emplace_back(s, p);
    return result;
}

static std::string join(const std::vector<std::string>& v, const std::string& sep)
{
    if (v.size() == 0) return std::string();
    std::ostringstream os;
    os << v[0];
    for (std::size_t i = 1; i < v.size(); i++) os << sep << v[i];
    return os.str();
}

template <class T>
std::ostream& operator <<(std::ostream& ost, const std::vector<T>& v)
{
    ost << "{";
    if (v.size() > 0) {
        for (std::size_t i = 0; i < v.size() - 1; i++) ost << v[i] << ", ";
        ost << v.back();
    }
    ost << "}";
    return ost;
}

template <class T, std::size_t N>
std::ostream& operator <<(std::ostream& ost, const std::array<T, N>& v)
{
    ost << "{";
    if (v.size() > 0) {
        for (std::size_t i = 0; i < v.size() - 1; i++) ost << v[i] << ", ";
        ost << v.back();
    }
    ost << "}";
    return ost;
}

template <class T>
T sum_of(const std::vector<T>& v)
{
    return std::accumulate(v.begin(), v.end(), T(0));
}

template <class T, std::size_t N>
T sum_of(const std::array<T, N>& v)
{
    return std::accumulate(v.begin(), v.end(), T(0));
}