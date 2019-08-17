#include "util.hpp"
#include "tictactoe.hpp"
#include "reversi.hpp"

using namespace std;

// build Python module

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

template <class state_t>
struct PythonState : state_t
{
    py::array_t<float> feature() const
    {
        std::array<int, 2> size = state_t::size();
        std::vector<float> f = state_t::feature();
        int channels = f.size() / (size[0] * size[1]);
        py::array_t<float> pf({channels, size[0], size[1]});
        std::memcpy(pf.mutable_data(), f.data(), sizeof(float) * f.size());
        return pf;
    }

    PythonState<state_t> copy() const
    {
        return PythonState<state_t>(*this);
    }
};

PYBIND11_MODULE(games, m)
{
    m.doc() = "implementation of game";

    using PyState0 = PythonState<TicTacToe::State>;

    py::class_<PyState0>(m, "TicTacToe")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState0::action2str, "action index to string")
    .def("str2action",    &PyState0::str2action, "string to action index")
    .def("__str__",       &PyState0::to_string, "string output")
    .def("copy",          &PyState0::copy, "deep copy")
    .def("clear",         &PyState0::clear, "initialize state")
    .def("legal_actions", &PyState0::legal_actions, "legal actions")
    .def("action_length", &PyState0::action_length, "the number of legal action labels")
    .def("play",          &PyState0::play, "state transition")
    .def("plays",         &PyState0::plays, "sequential state transition")
    .def("terminal",      &PyState0::terminal, "whether terminal TicTacToe or not")
    .def("reward",        &PyState0::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState0::feature, "input feature");
    
    using PyState1 = PythonState<Reversi::State>;

    py::class_<PyState1>(m, "Reversi")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState1::action2str, "action index to string")
    .def("str2action",    &PyState1::str2action, "string to action index")
    .def("__str__",       &PyState1::to_string, "string output")
    .def("copy",          &PyState1::copy, "deep copy")
    .def("clear",         &PyState1::clear, "initialize state")
    .def("legal_actions", &PyState1::legal_actions, "legal actions")
    .def("action_length", &PyState1::action_length, "the number of legal action labels")
    .def("play",          &PyState1::play, "state transition")
    .def("plays",         &PyState1::plays, "sequential state transition")
    .def("terminal",      &PyState1::terminal, "whether terminal state or not")
    .def("reward",        &PyState1::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState1::feature, "input feature");

};