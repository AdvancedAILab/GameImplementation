#include "util.hpp"
#include "tictactoe.hpp"
#include "reversi.hpp"
#include "animalshogi.hpp"
#include "go.hpp"
#include "geister.hpp"
#include "fliptictactoe.hpp"

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

    TicTacToe::init();
    using PyState0 = PythonState<TicTacToe::State>;

    py::class_<PyState0>(m, "TicTacToe")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState0::action2str, "action index to string")
    .def("str2action",    &PyState0::str2action, "string to action index")
    .def("str2path",      &PyState0::str2path, "string to ations list")
    .def("record_string", &PyState0::record_string, "string output of current path")
    .def("__str__",       &PyState0::to_string, "string output")
    .def("copy",          &PyState0::copy, "deep copy")
    .def("clear",         &PyState0::clear, "initialize state")
    .def("legal_actions", &PyState0::legal_actions, "legal actions")
    .def("best_actions",  &PyState0::best_actions, "best actions by minimax search")
    .def("action_length", &PyState0::action_length, "the number of legal action labels")
    .def("chance",        &PyState0::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState0::play, "state transition by action")
    .def("plays",         &PyState0::plays, "sequential state transition")
    .def("terminal",      &PyState0::terminal, "whether terminal TicTacToe or not")
    .def("reward",        &PyState0::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState0::feature, "input feature");

    Reversi::init();
    using PyState1 = PythonState<Reversi::State>;

    py::class_<PyState1>(m, "Reversi")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState1::action2str, "action index to string")
    .def("str2action",    &PyState1::str2action, "string to action index")
    .def("str2path",      &PyState1::str2path, "string to ations list")
    .def("record_string", &PyState1::record_string, "string output of current path")
    .def("__str__",       &PyState1::to_string, "string output")
    .def("copy",          &PyState1::copy, "deep copy")
    .def("clear",         &PyState1::clear, "initialize state")
    .def("legal_actions", &PyState1::legal_actions, "legal actions")
    .def("action_length", &PyState1::action_length, "the number of legal action labels")
    .def("chance",        &PyState1::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState1::play, "state transition by action")
    .def("plays",         &PyState1::plays, "sequential state transition")
    .def("terminal",      &PyState1::terminal, "whether terminal state or not")
    .def("reward",        &PyState1::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState1::feature, "input feature");

    AnimalShogi::init();
    using PyState2 = PythonState<AnimalShogi::State>;

    py::class_<PyState2>(m, "AnimalShogi")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState2::action2str, "action index to string")
    .def("str2action",    &PyState2::str2action, "string to action index")
    .def("str2path",      &PyState2::str2path, "string to ations list")
    .def("record_string", &PyState2::record_string, "string output of current path")
    .def("__str__",       &PyState2::to_string, "string output")
    .def("copy",          &PyState2::copy, "deep copy")
    .def("clear",         &PyState2::clear, "initialize state")
    .def("legal_actions", &PyState2::legal_actions, "legal actions")
    .def("action_length", &PyState2::action_length, "the number of legal action labels")
    .def("chance",        &PyState2::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState2::play, "state transition by action")
    .def("plays",         &PyState2::plays, "sequential state transition")
    .def("terminal",      &PyState2::terminal, "whether terminal state or not")
    .def("reward",        &PyState2::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState2::feature, "input feature");

    Go::init();
    using PyState3 = PythonState<Go::State>;

    py::class_<PyState3>(m, "Go")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState3::action2str, "action index to string")
    .def("str2action",    &PyState3::str2action, "string to action index")
    .def("str2path",      &PyState3::str2path, "string to ations list")
    .def("record_string", &PyState3::record_string, "string output of current path")
    .def("__str__",       &PyState3::to_string, "string output")
    .def("copy",          &PyState3::copy, "deep copy")
    .def("clear",         &PyState3::clear, "initialize state")
    .def("legal_actions", &PyState3::legal_actions, "legal actions")
    .def("action_length", &PyState3::action_length, "the number of legal action labels")
    .def("chance",        &PyState3::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState3::play, "state transition by action")
    .def("plays",         &PyState3::plays, "sequential state transition")
    .def("terminal",      &PyState3::terminal, "whether terminal state or not")
    .def("reward",        &PyState3::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState3::feature, "input feature");

    Geister::init();
    using PyState4 = PythonState<Geister::State>;

    py::class_<PyState4>(m, "Geister")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState4::action2str, "action index to string")
    .def("str2action",    &PyState4::str2action, "string to action index")
    .def("str2path",      &PyState4::str2path, "string to ations list")
    .def("record_string", &PyState4::record_string, "string output of current path")
    .def("__str__",       &PyState4::to_string, "string output")
    .def("copy",          &PyState4::copy, "deep copy")
    .def("clear",         &PyState4::clear, "initialize state")
    .def("legal_actions", &PyState4::legal_actions, "legal actions")
    .def("action_length", &PyState4::action_length, "the number of legal action labels")
    .def("chance",        &PyState4::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState4::play, "state transition by action")
    .def("plays",         &PyState4::plays, "sequential state transition")
    .def("terminal",      &PyState4::terminal, "whether terminal state or not")
    .def("reward",        &PyState4::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState4::feature, "input feature");

    FlipTicTacToe::init();
    using PyState5 = PythonState<FlipTicTacToe::State>;

    py::class_<PyState5>(m, "FlipTicTacToe")
    .def(pybind11::init<>(), "constructor")
    .def("action2str",    &PyState5::action2str, "action index to string")
    .def("str2action",    &PyState5::str2action, "string to action index")
    .def("str2path",      &PyState5::str2path, "string to ations list")
    .def("record_string", &PyState5::record_string, "string output of current path")
    .def("__str__",       &PyState5::to_string, "string output")
    .def("copy",          &PyState5::copy, "deep copy")
    .def("clear",         &PyState5::clear, "initialize state")
    .def("legal_actions", &PyState5::legal_actions, "legal actions")
    .def("action_length", &PyState5::action_length, "the number of legal action labels")
    .def("chance",        &PyState5::chance, "state transition by chance", py::arg("seed") = -1)
    .def("play",          &PyState5::play, "state transition by action")
    .def("plays",         &PyState5::plays, "sequential state transition")
    .def("terminal",      &PyState5::terminal, "whether terminal state or not")
    .def("reward",        &PyState5::reward, "terminal reward", py::arg("subjective") = false)
    .def("feature",       &PyState5::feature, "input feature");
};
