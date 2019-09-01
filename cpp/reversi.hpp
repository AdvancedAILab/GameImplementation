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
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {L_, L_};
        }

        void clear()
        {
            fill(board_.begin(), board_.end(), EMPTY);
            int mid = (L_ - 1) / 2;
            board_[xy2action(mid, mid)] = WHITE;
            board_[xy2action(mid, mid + 1)] = BLACK;
            board_[xy2action(mid + 1, mid)] = BLACK;
            board_[xy2action(mid + 1, mid + 1)] = WHITE;
            color_ = BLACK;
            score_.fill(2);
            record_.clear();
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

        void play(int action)
        {
            assert(legal(action));
            if (action != L_ * L_) {
                int flipped = flip_stones(action);
                board_[action] = color_;
                score_[color_] += 1 + flipped;
                score_[opponent(color_)] -= flipped;
            }
            color_ = opponent(color_);
            record_.push_back(action);
        }

        void plays(const string& s)
        {
            if (s.size() == 0) return;
            vector<string> ss = split(s, ' ');
            for (const string& s : ss) play(str2action(s));
        }

        bool terminal() const
        {
            // check if terminal state
            bool full = score_[0] + score_[1] == L_ * L_;
            bool perfect = score_[0] == 0 || score_[1] == 0;
            bool pass2 = record_.size() >= 2
                      && record_.back() == L_ * L_
                      && record_[record_.size() - 2] == L_ * L_;
            
            return full || perfect || pass2;
        }

        float reward(bool subjective = true) const
        {
            // 終端状態での勝敗による報酬を返す
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
            // 可能な行動リストを返す
            vector<int> actions;
            for (int i = 0; i < L_ * L_; i++) {
                if (legal(i)) actions.push_back(i);
            }
            if (actions.size() == 0) {
                actions.push_back(L_ * L_); // pass
            }
            return actions;
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

        array<int, 8> flip_counts(int action, int color = EMPTY) const
        {
            array<int, 8> counts;
            counts.fill(0);
            if (color == EMPTY) color = color_;
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
                        flipped = 0; // no stones will be sandwitched
                        break;
                    }
                    if (board_[pos] == color) break;
                    flipped++;
                }
                counts[d] = flipped;
            }
            return counts;
        }

        int flip_stones(int action)
        {
            auto counts = flip_counts(action);
            for (int d = 0; d < 8; d++) {
                int x = action2x(action);
                int y = action2y(action);
                for (int i = 0; i < counts[d]; i++) {
                    x += D2[d][0];
                    y += D2[d][1];
                    board_[xy2action(x, y)] = color_;
                }
            }
            return sum_of(counts);
        }
    };
}
