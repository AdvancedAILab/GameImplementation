#include "util.hpp"
#include "tictactoe.hpp"
#include "reversi.hpp"

using namespace std;

// executable

int main(int argc, char *argv[])
{
    srand(0);
    for (int i = 0; i < 10; i++) {
        TicTacToe::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }

    for (int i = 0; i < 10; i++) {
        Reversi::State state;
        while (!state.terminal()) {
            auto actions = state.legal_actions();
            cerr << state.to_string() << endl;
            state.play(actions[rand() % actions.size()]);
        }
        cerr << state.to_string() << endl;
        cerr << "reward = " << state.reward(false) << endl;
    }
}