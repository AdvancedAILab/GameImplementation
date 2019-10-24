#pragma once

#include <map>
#include <random>


#include "util.hpp"
#include "boardgame.hpp"

using namespace std;

namespace Geister
{
    const string X = "ABCDEF";
    const string Y = "123456";
    const int BLUE = 0;
    const int RED = 1;
    const string P = "_BRbr";
    const vector<vector<string>> OPOS = {
        // 初期に駒を配置する位置
        {"B1", "C1", "D1", "E1", "B2", "C2", "D2", "E2"},
        {"B5", "C5", "D5", "E5", "B6", "C6", "D6", "E6"}
    };
    const vector<vector<array<int, 2>>> GPOS = {
        // 到達して勝ちになる位置
        {{-1, 5}, {6, 5}},
        {{-1, 0}, {6, 0}}
    };

    vector<array<long long, 16>> POSITION_KEY;

    inline void init() {
        mt19937_64 mt(0);
        for (int pos = 0; pos < 6 * 6; pos++) {
            array<long long, 16> keys;
            for (int i = 0; i < 8 * 2; i++) {
                keys[i] = mt();
            }
            POSITION_KEY.push_back(keys);
        }
    }

    struct State
    {
        const int L_ = 6;
        const int B_ = L_ * L_;
        vector<int> board_;
        int color_;
        int win_color_;
        array<int, 4> piece_cnt_;
        array<int, 16> piece_position_;
        vector<int> board_index_;
        long long key_;
        map<long long, int> prev_keys_;
        vector<int> record_;

        State()
        {
            board_.resize(B_);
            board_index_.resize(B_);
            clear();
        }

        State(const State& s):
        board_(s.board_),
        color_(s.color_),
        win_color_(s.win_color_),
        piece_cnt_(s.piece_cnt_),
        piece_position_(s.piece_position_),
        board_index_(s.board_index_),
        key_(s.key_),
        prev_keys_(s.prev_keys_),
        record_(s.record_) {}

        array<int, 2> size() const
        {
            return {L_, L_};
        }

        void clear(long long seed = 0)
        {
            fill(board_.begin(), board_.end(), -1);
            color_ = BLACK;
            win_color_ = -1;
            piece_cnt_.fill(0);
            piece_position_.fill(-1);
            fill(board_index_.begin(), board_index_.end(), -1);
            key_ = 0;
            prev_keys_.clear();
            record_.clear();

            // randomly setting original position
            mt19937_64 mt(seed);
            for (int c = 0; c < 2; c++) {
                array<int, 8> seq;
                for (int i = 0; i < 8; i++) seq[i] = i;
                shuffle(seq.begin(), seq.end(), mt);
                for (int t = 0; t < 2; t++) {
                    for (int i = t * 4; i < (t + 1) * 4; i++) {
                        int index = seq[i];
                        int piece = colortype2piece(c, t);
                        int pos = str2position(OPOS[c][index]);
                        put_piece(piece, pos, c * 8 + index);
                    }
                }
            }

            long long key = key_ ^ color_;
            prev_keys_[key] += 1;
        }

        int colortype2piece(int c, int t) const
        {
            return c * 2 + t;
        }

        int piece2color(int p) const
        {
            return p == -1 ? -1 : p / 2;
        }

        int piece2type(int p) const
        {
            return p == -1 ? -1 : p % 2;
        }

        int xy2position(int x, int y) const
        {
            return x * L_ + y;
        }

        int position2x(int pos) const
        {
            return pos / L_;
        }

        int position2y(int pos) const
        {
            return pos % L_;
        }

        int fromdirection2to(int pos_from, int d) const
        {
            int x_to = position2x(pos_from) + D2[d][0];
            int y_to = position2y(pos_from) + D2[d][1];
            return xy2position(x_to, y_to);
        }

        int fromdirection2action(int pos_from, int d) const
        {
            return d * B_ + pos_from;
        }

        int action2from(int action) const
        {
            return action % B_;
        }

        int action2direction(int action) const
        {
            return action / B_;
        }

        int onboard_next(int pos_from, int d) const
        {
            // check position after moving
            int x_to = position2x(pos_from) + D2[d][0];
            int y_to = position2y(pos_from) + D2[d][1];
            return onboard_xy(x_to, y_to, L_);
        }

        char piece2char(int piece) const
        {
            return P[piece + 1];
        }

        int char2piece(char c) const
        {
            return (int)P.find(c) - 1;
        }

        string position2str(int pos) const
        {
            if (!onboard(pos, L_)) return "**";
            ostringstream oss;
            oss << X[position2x(pos)] << Y[position2y(pos)];
            return oss.str();
        }

        int str2position(const string& s) const
        {
            if (s == "**") return -1;
            int x = X.find(s[0]);
            int y = Y.find(s[1]);
            return xy2position(x, y);
        }

        string action2str(int action) const
        {
            int pos_from = action2from(action);
            int d = action2direction(action);
            int pos_to = -1;
            if (onboard_next(pos_from, d)) {
                pos_to = fromdirection2to(pos_from, d);
            }
            return position2str(pos_from) + position2str(pos_to);
        }

        int str2action(const string& s) const
        {
            int pos_from = str2position(s.substr(0, 2));
            int pos_to = str2position(s.substr(2, 2));
            int x_from = position2x(pos_from);
            int y_from = position2y(pos_from);
            int dx, dy;

            if (pos_to == -1) {
                // reached the nearest goal
                for (int c = 0; c < 2; c++) {
                    for (auto gxy : GPOS[c]) {
                        dx = gxy[0] - x_from;
                        dy = gxy[1] - y_from;
                        int dr = abs(dx) + abs(dy); // manhattan distance
                        if (dr == 1) break;
                    }
                }
            } else {
                dx = position2x(pos_to) - x_from;
                dy = position2y(pos_to) - y_from;
            }

            int d = -1;
            for (int dd = 0; dd < 4; dd++) {
                if (D2[dd][0] == dx && D2[dd][1] == dy) {
                    d = dd;
                    break;
                }
            }

            return fromdirection2action(pos_from, d);
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
            for (int y = 0; y < L_; y++) oss << Y[y];
            oss << endl;
            for (int x = 0; x < L_; x++) {
                oss << X[x] << " ";
                for (int y = 0; y < L_; y++) {
                    oss << piece2char(board_[xy2position(x, y)]);
                }
                oss << endl;
            }
            oss << "record = " << record_string();
            return oss.str();
        }

        void put_piece(int piece, int pos, int piece_index)
        {
            board_[pos] = piece;
            piece_position_[piece_index] = pos;
            board_index_[pos] = piece_index;
            piece_cnt_[piece] += 1;
            key_ ^= POSITION_KEY[pos][piece_index];
        }

        void remove_piece(int pos)
        {
            int piece = board_[pos];
            board_[pos] = -1;
            int piece_index = board_index_[pos];
            board_index_[pos] = -1;
            piece_position_[piece_index] = -1;
            piece_cnt_[piece] -= 1;
            key_ ^= POSITION_KEY[pos][piece_index];
        }

        void move_piece(int pos_from, int pos_to)
        {
            int piece = board_[pos_from];
            board_[pos_from] = -1;
            board_[pos_to] = piece;
            int piece_index = board_index_[pos_from];
            board_index_[pos_from] = -1;
            board_index_[pos_to] = piece_index;
            piece_position_[piece_index] = pos_to;
            key_ ^= POSITION_KEY[pos_from][piece_index];
            key_ ^= POSITION_KEY[pos_to][piece_index];
        }

        bool goal(int color, int pos_from, int d) const
        {
            int x_to = position2x(pos_from) + D2[d][0];
            int y_to = position2y(pos_from) + D2[d][1];
            for (auto& gpos : GPOS[color]) {
                if (x_to == gpos[0] && y_to == gpos[1]) return true;
            }
            return false;
        }

        void chance(int seed=-1) {}

        void play(int action)
        {
            assert(legal(action));
            int pos_from = action2from(action);
            int d = action2direction(action);

            if (!onboard_next(pos_from, d)) {
                // finish by reaching goal position
                remove_piece(pos_from);
                win_color_ = color_;
            } else {
                int pos_to = fromdirection2to(pos_from, d);
                int piece_cap = board_[pos_to];
                if (piece_cap != -1) {
                    // capture opponent piece
                    remove_piece(pos_to);
                    if (piece_cnt_[piece_cap] == 0) {
                        if (piece2type(piece_cap) == BLUE) {
                            // win by capturing all opponent blue pieces
                            win_color_ = color_;
                        } else {
                            // lose by capturing all opponent red pieces
                            win_color_ = opponent(color_);
                        }
                    }
                }
                move_piece(pos_from, pos_to);
            }

            color_ = opponent(color_);

            // repetition check
            long long key = key_ ^ color_;
            prev_keys_[key] += 1;

            if (prev_keys_[key] >= 3) { // draw
                win_color_ = 2;
            }

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
            return win_color_ != -1;
        }

        float reward(bool subjective = true) const
        {
            int r = 0;
            if (win_color_ == color_) r = 1;
            else if (win_color_ == opponent(color_)) r = -1;
            return subjective && color_ == WHITE ? -r : r;
        }

        bool legal(int action) const
        {
            int pos_from = action2from(action);
            int d = action2direction(action);

            int piece = board_[pos_from];
            int c = piece2color(piece);
            int t = piece2type(piece);
            if (c != color_) return false;

            return _legal(t, pos_from, d);
        }

        bool _legal(int t, int pos_from, int d) const
        {
            if (onboard_next(pos_from, d)) {
                int pos_to = fromdirection2to(pos_from, d);
                int piece_cap = board_[pos_to];
                return piece2color(piece_cap) != color_;
            } else {
                return t == BLUE && goal(color_, pos_from, d);
            }
        }

        vector<int> legal_actions() const
        {
            vector<int> actions;
            for (int i = color_ * 8; i < (color_ + 1) * 8; i++) {
                int pos = piece_position_[i];
                if (pos == -1) continue;

                int t = piece2type(board_[pos]);
                for (int d = 0; d < 4; d++) {
                    if (_legal(t, pos, d)) {
                        int action = fromdirection2action(pos, d);
                        actions.push_back(action);
                    }
                }
            }

            return actions;
        }

        int action_length() const
        {
            return 4 * B_;
        }

        vector<float> feature() const
        {
            vector<float> f(2 * L_ * L_, 0.0f);
            int col = color_, opp = opponent(color_);

            int p[4] = {
                colortype2piece(col, 0), colortype2piece(col, 1),
                colortype2piece(opp, 0), colortype2piece(opp, 1)
            };
            float c[4] = {
                log2f((float)piece_cnt_[p[0]]), log2f((float)piece_cnt_[p[1]]),
                log2f((float)piece_cnt_[p[2]]), log2f((float)piece_cnt_[p[3]])
            };

            for (int pos = 0; pos < B_; pos++) {
                if (color_ == BLACK) f[pos] = 1;
                if (board_[pos] == p[0]) f[pos + B_] = 1;
                if (board_[pos] == p[1]) f[pos + 2 * B_] = 1;
                if (board_[pos] == p[2] || board_[pos] == p[3]) f[pos + 3 * B_] = 1;
                f[pos + 4 * B_] = c[0];
                f[pos + 5 * B_] = c[1];
                f[pos + 6 * B_] = c[2];
                f[pos + 7 * B_] = c[3];
            }

            return f;
        }
    };
}
