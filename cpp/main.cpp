#include "util.hpp"
#include "tictactoe.hpp"
#include "reversi.hpp"
#include "animalshogi.hpp"
#include "go.hpp"

using namespace std;

// executable

template <class callback_t>
string map_list(const vector<int>& v, const callback_t& callback)
{
    vector<string> sv;
    for (int val : v) sv.push_back(callback(val));
    return join(sv, " ");
}

int main(int argc, char *argv[])
{
    srand(0);

    TicTacToe::init();
    for (int i = 0; i < 10; i++) {
        TicTacToe::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            cerr << state.to_string() << endl;
            cerr << actions << endl;
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }

    Reversi::init();
    for (int i = 0; i < 10; i++) {
        Reversi::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            cerr << state.to_string() << endl;
            cerr << actions << endl;
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }

    AnimalShogi::init();
    for (int i = 0; i < 10; i++) {
        AnimalShogi::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            cerr << state.to_string() << endl;
            cerr << actions << endl;
            cerr << map_list(actions, AnimalShogi::State::action2str) << endl;
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }

    Go::init();
    for (int i = 0; i < 10; i++) {
        Go::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            cerr << state.to_string() << endl;
            cerr << actions << endl;
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }
}