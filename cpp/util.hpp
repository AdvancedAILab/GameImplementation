#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <algorithm>

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

#ifdef PY

// build for Python module

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

template <class state_t>
struct PythonState : state_t
{
    pybind11::array_t<float> feature() const
    {
        std::array<int, 2> size = state_t::size();
        std::vector<float> f = state_t::feature();
        int channels = f.size() / size[0] * size[1];
        pybind11::array_t<float> pf({channels, size[0], size[1]});
        std::memcpy(pf.mutable_data(), f.data(), sizeof(float) * f.size());
        return pf;
    }

    PythonState<state_t> copy() const
    {
        return PythonState<state_t>(*this);
    }
};

#endif