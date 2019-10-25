#pragma once

#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

namespace Reversi
{
    const string X = "ABCDEFGH";
    const string Y = "12345678";
    const string C = "XO.";

    inline void init() {}

    struct State
    {
        int L_ = 6;
        vector<int> board_;
        int color_;
        array<int, 2> score_;
        vector<array<int, 8>> flipped_counts_; // for undo
        vector<int> record_;

        State()
        {
            board_.resize(L_ * L_);
            clear();
        }

        State(const State& s):
        L_(s.L_),
        board_(s.board_),
        color_(s.color_),
        score_(s.score_),
        flipped_counts_(s.flipped_counts_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {L_, L_};
        }

        void clear()
        {
            fill(board_.begin(), board_.end(), EMPTY);
            color_ = BLACK;
            record_.clear();

            // original state
            int mid = (L_ - 1) / 2;
            board_[xy2action(mid, mid)] = WHITE;
            board_[xy2action(mid, mid + 1)] = BLACK;
            board_[xy2action(mid + 1, mid)] = BLACK;
            board_[xy2action(mid + 1, mid + 1)] = WHITE;
            score_.fill(2);
        }

        string action2str(int action) const
        {
            if (action == L_ * L_) return "PASS";
            ostringstream oss;
            oss << X[action2x(action)] << Y[action2y(action)];
            return oss.str();
        }

        int str2action(const string& s) const
        {
            return xy2action(find(X, s[0]), find(Y, s[1]));
        }

        string path2str(const vector<int>& path) const {
            vector<string> ss;
            for (int action : path) ss.push_back(action2str(action));
            return join(ss, " ");
        }

        vector<int> str2path(const string& s) const {
            vector<int> path;
            if (s.size() == 0) return path;
            for (const string& as : split(s, ' ')) path.push_back(str2action(as));
            return path;
        }

        string record_string() const
        {
            return path2str(record_);
        }

        string to_string() const
        {
            ostringstream oss;
            oss << "  ";
            for (int x = 0; x < L_; x++) oss << X[x];
            oss << endl;
            for (int y = 0; y < L_; y++) {
                oss << Y[y] << " ";
                for (int x = 0; x < L_; x++) {
                    oss << C[board_[xy2action(x, y)]];
                }
                oss << endl;
            }
            oss << score_[0] << " - " << score_[1] << endl;
            oss << "record = " << record_string();
            return oss.str();
        }

        void chance(int seed=-1) {}

        void play(int action)
        {
            assert(legal(action));
            if (action != L_ * L_) {
                auto counts = flip_counts(action);
                int flipped = flip_stones(action, counts);
                board_[action] = color_;
                score_[color_] += 1 + flipped;
                score_[opponent(color_)] -= flipped;
                flipped_counts_.push_back(counts);
            }
            color_ = opponent(color_);
            record_.push_back(action);
        }

        void unchance() {}

        void undo()
        {
            assert(!record_.empty());
            int action = record_.back();
            if (action != L_ * L_) {
                auto counts = flipped_counts_.back();
                int flipped = flip_stones(action, counts);
                board_[action] = EMPTY;
                score_[opponent(color_)] -= 1 + flipped;
                score_[color_] += flipped;
                flipped_counts_.pop_back();
            }
            color_ = opponent(color_);
            record_.pop_back();
        }

        void plays(const string& s)
        {
            if (s.size() == 0) return;
            vector<string> ss = split(s, ' ');
            for (const string& s : ss) play(str2action(s));
        }

        bool terminal() const
        {
            bool full = score_[0] + score_[1] == L_ * L_;
            bool perfect = score_[0] == 0 || score_[1] == 0;
            bool pass2 = record_.size() >= 2
                      && record_.back() == L_ * L_
                      && record_[record_.size() - 2] == L_ * L_;

            return full || perfect || pass2;
        }

        float reward(bool subjective = true) const
        {
            int sc = score(subjective);
            if (sc > 0) return 1;
            else if (sc < 0) return -1;
            return 0;
        }

        bool legal(int action) const
        {
            if (onboard(action, L_)) {
                return sum_of(flip_counts(action)) > 0;
            } else {
                if (action != L_ * L_) return false;
                for (int i = 0; i < L_ * L_; i++) {
                    if (sum_of(flip_counts(i)) > 0) return false;
                }
                return true;
            }
        }

        vector<int> legal_actions() const
        {
            vector<int> actions;
            for (int i = 0; i < L_ * L_; i++) {
                if (legal(i)) actions.push_back(i);
            }
            if (actions.size() == 0) {
                actions.push_back(L_ * L_); // pass
            }
            return actions;
        }

        pair<vector<int>, float> best_actions() const
        {
            State s(*this);
            return alpha_beta_search(s);
        }

        int action_length() const
        {
            return L_ * L_ + 1;
        }

        vector<float> feature() const
        {
            vector<float> f(2 * L_ * L_, 0.0f);
            for (int pos = 0; pos < L_ * L_; pos++) {
                if      (board_[pos] == color_)           f[pos] = 1;
                else if (board_[pos] == opponent(color_)) f[pos + L_ * L_] = 1;
            }
            return f;
        }

        int score(bool subjective = true) const
        {
            int diff = score_[0] - score_[1];
            return (subjective && color_ == WHITE) ? -diff : diff;
        }

        int action2x(int action) const
        {
            return action % L_;
        }

        int action2y(int action) const
        {
            return action / L_;
        }

        int xy2action(int x, int y) const
        {
            return y * L_ + x;
        }

        array<int, 8> flip_counts(int action) const
        {
            array<int, 8> counts;
            counts.fill(0);
            if (!onboard(action, L_)) return counts;
            if (board_[action] != EMPTY) return counts;
            for (int d = 0; d < 8; d++) {
                int flipped = 0;
                int x = action2x(action);
                int y = action2y(action);
                while (1) {
                    x += D2[d][0];
                    y += D2[d][1];
                    int pos = xy2action(x, y);
                    if (!onboard_xy(x, y, L_) || board_[pos] == EMPTY) {
                        flipped = 0; // no stones will be sandwiched
                        break;
                    }
                    if (board_[pos] == color_) break;
                    flipped++;
                }
                counts[d] = flipped;
            }
            return counts;
        }

        int flip_stones(int action, const array<int, 8>& counts)
        {
            for (int d = 0; d < 8; d++) {
                int x = action2x(action);
                int y = action2y(action);
                for (int i = 0; i < counts[d]; i++) {
                    x += D2[d][0];
                    y += D2[d][1];
                    assert(onboard_xy(x, y, L_));
                    int pos = xy2action(x, y);
                    board_[pos] = opponent(board_[pos]);
                }
            }
            return sum_of(counts);
        }
    };
}
