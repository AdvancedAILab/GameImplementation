#pragma once

#include <random>

#include "tictactoe.hpp"

using namespace std;

namespace FlipTicTacToe
{
    inline void init() {}

    struct State : public TicTacToe::State
    {
        using base_t = TicTacToe::State;
        array<int, 2> score_;
        vector<pair<int, int>> flip_record_;

        State():
        base_t(),
        score_(),
        flip_record_() {}

        State(const State& s):
        base_t(s),
        score_(s.score_),
        flip_record_(s.flip_record_) {}

        void clear()
        {
            base_t::clear();
            score_.fill(0);
            flip_record_.clear();
        }

        void chance(int seed=-1)
        {
            if (record_.empty()) return;
            if (seed == -1) {
                random_device rd;
                seed = rd();
            }
            mt19937 mt(seed);
            const int l = base_t::L_;
            const int b = l * l;
            int pos0 = mt() % b;
            int pos1 = mt() % (b - 1);
            if (pos1 >= pos0) pos1++;

            swap(base_t::board_[pos0], base_t::board_[pos1]);
            flip_record_.push_back(make_pair(pos0, pos1));

            // winning check
            for (int c = 0; c < 2; c++) {
                score_[c] = 0;
                for (int x = 0; x < base_t::L_; x++) {
                    int sum[2] = {0, 0};
                    for (int y = 0; y < base_t::L_; y++) sum[0] += base_t::board_[xy2action(x, y)] == c ? 1 : 0;
                    for (int y = 0; y < base_t::L_; y++) sum[1] += base_t::board_[xy2action(y, x)] == c ? 1 : 0;
                    for (int i = 0; i < 2; i++) if (sum[i] == base_t::L_) score_[c]++;
                }
                int sum[2] = {0, 0};
                for (int x = 0; x < base_t::L_; x++) sum[0] += base_t::board_[xy2action(x, x)] == c ? 1 : 0;
                for (int x = 0; x < base_t::L_; x++) sum[1] += base_t::board_[xy2action(x, base_t::L_ - 1 - x)] == c ? 1 : 0;
                for (int i = 0; i < 2; i++) if (sum[i] == base_t::L_) score_[c]++;
            }
            if      (score_[0] > score_[1]) base_t::win_color_ = BLACK;
            else if (score_[0] <= score_[1]) {
                if (score_[1] > 0 || int(flip_record_.size()) == L_ * L_) base_t::win_color_ = WHITE;
            }
        }

        void play(int action)
        {
            assert(legal(action));
            board_[action] = color_;

            color_ = opponent(color_);
            record_.push_back(action);
        }

        void unchance()
        {
            if (record_.empty()) return;
            assert(!flip_record_.empty());
            auto flipped = flip_record_.back();
            flip_record_.pop_back();
            swap(base_t::board_[flipped.first], base_t::board_[flipped.second]);
            win_color_ = EMPTY;
        }

        void undo()
        {
            assert(!record_.empty());
            int action = record_.back();
            record_.pop_back();
            color_ = opponent(color_);
            board_[action] = EMPTY;
        }

        bool terminal() const
        {
            return score_[0] + score_[1] > 0 || int(flip_record_.size()) == L_ * L_;
        }

        vector<float> feature() const
        {
            const int b = base_t::L_ * base_t::L_;
            vector<float> f(3 * b, 0.0f);
            for (int pos = 0; pos < b; pos++) {
                if (board_[pos] == color_)           f[pos] = 1;
                if (board_[pos] == opponent(color_)) f[pos + b] = 1;
                if (color_ == BLACK)                 f[pos + b * 2] = 1;
            }
            return f;
        }
    };
}
