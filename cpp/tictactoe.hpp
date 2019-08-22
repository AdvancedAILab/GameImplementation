#pragma once

#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

namespace TicTacToe
{
    const string X = "ABC";
    const string Y = "123";
    const string C = "OX.";

    inline void init() {}

    struct State
    {
        const int L_ = 3;
        vector<int> board_;
        int color_;
        int win_color_;
        vector<int> record_;

        State()
        {
            board_.resize(L_ * L_);
            clear();
        }

        State(const State& s):
        board_(s.board_),
        color_(s.color_),
        win_color_(s.win_color_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {L_, L_};
        }

        void clear()
        {
            fill(board_.begin(), board_.end(), EMPTY);
            color_ = BLACK;
            win_color_ = EMPTY;
            record_.clear();
        }

        string action2str(int action) const
        {
            ostringstream oss;
            oss << X[action2x(action)] << Y[action2y(action)];
            return oss.str();
        }

        int str2action(const string& s) const
        {
            return xy2action(find(X, s[0]), find(Y, s[1]));
        }

        string record_string() const
        {
            vector<string> ss;
            for (int action : record_) {
                ss.push_back(action2str(action));
            }
            return join(ss, " ");
        }

        string to_string() const
        {
            ostringstream oss;
            oss << "  ";
            for (int y = 0; y < L_; y++) oss << Y[y];
            oss << endl;
            for (int x = 0; x < L_; x++) {
                oss << X[x] << " ";
                for (int y = 0; y < L_; y++) {
                    oss << C[board_[xy2action(x, y)]];
                }
                oss << endl;
            }
            oss << "record = " << record_string();
            return oss.str();
        }

        void play(int action)
        {
            assert(legal(action));
            board_[action] = color_;
            int ax = action2x(action), ay = action2y(action);

            // 一列揃ったか調べる
            int xsum = 0, ysum = 0;
            for (int y = 0; y < L_; y++) xsum += board_[xy2action(ax, y)] == color_ ? 1 : 0;
            for (int x = 0; x < L_; x++) ysum += board_[xy2action(x, ay)] == color_ ? 1 : 0;
            if (xsum == L_ || ysum == L_) win_color_ = color_;

            if (ax == ay) {
                int diagsum = 0;
                for (int x = 0; x < L_; x++) diagsum += board_[xy2action(x, x)] == color_ ? 1 : 0;
                if (diagsum == L_) win_color_ = color_;
            }
            if (ax == L_ - 1 - ay) {
                int diagsum = 0;
                for (int x = 0; x < L_; x++) diagsum += board_[xy2action(x, L_ - 1 - x)] == color_ ? 1 : 0;
                if (diagsum == L_) win_color_ = color_;
            }

            color_ = opponent(color_);
            record_.push_back(action);
        }

        void plays(const string& s)
        {
            vector<string> ss = split(s, ' ');
            for (const string& s : ss) play(str2action(s));
        }

        bool terminal() const
        {
            // 終端状態かどうか返す
            return win_color_ != EMPTY || int(record_.size()) == L_ * L_;
        }

        float reward(bool subjective = true) const
        {
            int robj = win_color_ == BLACK ? 1 : (win_color_ == WHITE ? -1 : 0);
            return (subjective && color_ == WHITE) ? -robj : robj;
        }

        bool legal(int action) const
        {
            return action >= 0 && action < L_ * L_ && board_[action] == EMPTY;
        }

        vector<int> legal_actions() const
        {
            // 可能な行動リストを返す
            vector<int> actions;
            for (int i = 0; i < L_ * L_; i++) {
                if (legal(i)) actions.push_back(i);
            }
            return actions;
        }

        int action_length() const
        {
            return L_ * L_;
        }

        vector<float> feature() const
        {
            vector<float> f(2 * L_ * L_, 0.0f);
            for (int x = 0; x < L_; x++) {
                for (int y = 0; y < L_; y++)  {
                    int a = xy2action(x, y);
                    if      (board_[a] == color_)           f[a] = 1;
                    else if (board_[a] == opponent(color_)) f[L_ * L_ + a] = 1;
                }
            }
            return f;
        }

        int action2x(int action) const
        {
            return action / L_;
        }

        int action2y(int action) const
        {
            return action % L_;
        }

        int xy2action(int x, int y) const
        {
            return x * L_ + y;
        }
    };
}